
#include "mutosim.h"

#include "calculation.h"
#include "cli_options.h"
#include "cli_options_format.h"
#include "logger.h"
#include "model/analytical.h"
#include "model/test.h"
#include "scenarios/single_overflow.h"
#include "scenarios/topology_based.h"
#include "simulation/group.h"
#include "simulation/source_stream/source_stream.h"
#include "simulation/world.h"
#include "topology_parser.h"
#include "types/types.h"
#include "types/types_format.h"
#include "utils.h"

#include <boost/program_options/parsers.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/utility/functional.hpp>
#include <range/v3/view/map.hpp>
#include <sys/ioctl.h>

namespace rng = ranges;

void
print_state(const std::vector<bool> &states)
{
  winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  auto              width = ws.ws_col - 10;
  const auto        rows = states.size() / width + 2;
  std::stringstream str;
  int               finished = 0;
  int               current = 0;

  str << "     ";
  for (bool state : states)
  {
    str << (state ? "\033[48;2;255;255;0m " : "\033[48;2;155;155;155m ");
    finished += state;
    if (++current % width == 0)
    {
      str << "\033[0m\n     ";
    }
  }
  str << "\033[0m";
  if (finished > 0)
  {
    for (auto row = 0u; row < rows; ++row)
    {
      print("\033[2K\033[1F"); // clear current and above line
    }
  }
  // print("\033[H\033[J"); // clear ANSI terminal
  println("[Main]: Finished {}/{}: \n{}", finished, states.size(), str.str());
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
nlohmann::json
run_scenarios(std::vector<ScenarioSettings> &scenarios, const CLIOptions &cli)
{
  nlohmann::json    global_stats = {};
  std::vector<bool> scenarios_state(scenarios.size());

  print_state(scenarios_state);

  sort(begin(scenarios), end(scenarios), [](const auto &s1, const auto &s2) {
    return s1.a > s2.a;
  });

#if !SINGLE_THREADED
#pragma omp parallel for schedule(guided, 8) if (cli.parallel)
#endif
  for (auto i = 0ul; i < scenarios.size(); ++i)
  {
    debug_println(
        fg(fmt::color::green),
        "Scenario: {}, file: {}",
        scenarios[i].name,
        scenarios[i].filename);
    nlohmann::json analytical_stats;
    switch (scenarios[i].mode)
    {
      case Mode::Simulation:
      {
        run_scenario(scenarios[i], cli.duration, cli.use_random_seed, true);
        break;
      }
      case Mode::Analytic:
      {
        Model::analytical_computations(scenarios[i]);
        break;
      }
      case Mode::Test:
      {
        break;
      }
    }

    auto A_str = std::to_string(ts::get(scenarios[i].A));
    auto filename = scenarios[i].filename;
#if !SINGLE_THREADED
#pragma omp critical
#endif
    {
      if (global_stats.find(filename) == end(global_stats))
      {
        global_stats[filename]["_scenario"] = scenarios[i].json;
      }
      auto &scenario_stats = global_stats[filename][A_str];
      scenario_stats = concatenate(scenario_stats, scenarios[i].stats);
      scenario_stats["_a"] = ts::get(scenarios[i].a);
      scenario_stats["_A"] = ts::get(scenarios[i].A);

      scenarios_state[i] = true;
      print_state(scenarios_state);
    }
  }
  return global_stats;
}
//----------------------------------------------------------------------
std::vector<std::string>
find_all_scenario_files(const std::string &path)
{
  std::vector<std::string> list_of_scenario_files;
  if (fs::exists(path) && fs::is_directory(path))
  {
    fs::recursive_directory_iterator iter(path);
    fs::recursive_directory_iterator end;

    while (iter != end)
    {
      if (is_regular_file(iter->path()) && iter->path().extension() == ".json")
      {
        list_of_scenario_files.push_back(iter->path().string());
      }

      std::error_code ec;
      iter.increment(ec);
      if (ec)
      {
        std::cerr << "Error While Accessing : " << iter->path().string()
                  << " :: " << ec.message() << '\n';
      }
    }
  }
  return list_of_scenario_files;
}
//----------------------------------------------------------------------

void
load_scenarios_from_files(
    std::vector<ScenarioSettings> & scenarios,
    const std::vector<std::string> &scenario_files,
    const CLIOptions &              cli)
{
  for (const auto &scenario_file : scenario_files)
  {
    debug_println(fg(fmt::color::yellow), "Parsing '{}' file.", scenario_file);
    const auto [topology, topology_json] =
        Config::parse_topology_config(scenario_file, cli.append_scenario_files);
    // Config::dump(topology);
    for (auto A = cli.A_start; A < cli.A_stop; A += cli.A_step)
    {
      if (contains(cli.modes, Mode::Simulation))
      {
        // TODO(PW): get rid of duplicated code
        for (int i = 0; i < cli.count; ++i)
        {
          auto scenario = prepare_scenario_local_group_A(topology, A);
          scenario.name += fmt::format(" A={}", A);
          scenario.mode = Mode::Simulation;

          std::string filename = scenario_file;
          auto &      appended_filenames = cli.append_scenario_files;
          if (!appended_filenames.empty())
          {
            filename = fmt::format(
                "{};{}", scenario_file, join(appended_filenames, ";"));
          }
          scenario.filename = filename;
          scenario.json = topology_json;
          scenarios.emplace_back(std::move(scenario));
        }
      }
      if (contains(cli.modes, Mode::Analytic))
      {
        for (const auto &model : cli.analytic_models)
        {
          auto scenario = prepare_scenario_local_group_A(topology, A);

          scenario.name += fmt::format(" A={}, analytic model={}", A, model);
          scenario.mode = Mode::Analytic;
          scenario.analytic_model = model;

          scenario.layers_types =
              Model::determine_layers_types(scenario.topology);
          if (rng::any_of(
                  scenario.layers_types | rng::view::values,
                  [](auto layer_type) {
                    return layer_type == Model::LayerType::Unknown;
                  }))
          {
            println("Couldn't determine analytic model for each layer.");
            continue;
          }

          std::string filename = scenario_file;
          auto &      appended_filenames = cli.append_scenario_files;
          if (!appended_filenames.empty())
          {
            filename = fmt::format(
                "{};{}", scenario_file, join(appended_filenames, ";"));
          }
          scenario.filename = fmt::format("{};analytic;{}", filename, model);
          scenario.json = topology_json;
          scenarios.emplace_back(std::move(scenario));
        }
      }
    }
  }
}

//----------------------------------------------------------------------
void
save_json(
    const nlohmann::json &j,
    const fs::path &      output_dir,
    const fs::path &      filename)
{
  std::filesystem::path output_file{output_dir};
  output_file /= filename;
  create_directories(output_file.parent_path());
  std::ofstream stats_file(
      output_file.string(), std::ios_base::out | std::ios_base::binary);
  if (output_file.extension() == ".ubjson")
  {
    auto data = nlohmann::json::to_ubjson(j, true, true);
    stats_file.write(
        reinterpret_cast<const char *>(data.data()),
        static_cast<long>(data.size()));
  }
  else if (output_file.extension() == ".cbor")
  {
    auto data = nlohmann::json::to_cbor(j);
    stats_file.write(
        reinterpret_cast<const char *>(data.data()),
        static_cast<long>(data.size()));
  }
  else
  {
    stats_file << j.dump(0);
  }
}
//----------------------------------------------------------------------
void
print_stats(const std::vector<ScenarioSettings> &scenarios)
{
  for (auto &scenario : scenarios)
  {
    print("\n[Main] {:-^100}\n", scenario.name);
    scenario.world->print_stats();

    if (scenario.do_after)
    {
      scenario.do_after();
    }
    print("[Main] {:^^100}\n", scenario.name);
  }
}
//----------------------------------------------------------------------
int
main(int argc, char *argv[])
{
  setlocale(LC_NUMERIC, "en_US.UTF-8");

  namespace po = boost::program_options;

  auto                               desc = prepare_options_description();
  po::positional_options_description p;
  p.add("scenario-file", -1);

  po::variables_map vm;
  {
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    std::ifstream cfg_file{"mutosim.cfg"};
    po::store(po::parse_config_file(cfg_file, desc), vm);
    po::notify(vm);
  }

  const auto cli = parse_args(vm);

  if (cli.help)
  {
    print("{}", desc);
    return 0;
  }
  println("Modes: {}", cli.modes);

  if (contains(cli.modes, Mode::Test))
  {
    Model::test();
    return 0;
  }
  if (contains(cli.modes, Mode::Analytic))
  {
    println("Analytic models: {}", cli.analytic_models);
  }

  if (!std::all_of(
          begin(cli.scenario_files),
          end(cli.scenario_files),
          [](const std::string &file) {
            namespace fs = std::filesystem;
            if (auto path = fs::path{file}; exists(path))
            {
              return true;
            }
            else
            {
              println("[Main] Scenario file {} doesn't exists.", path.string());
              return false;
            }
          }))
  {
    return ENOENT;
  }

  std::vector<ScenarioSettings> scenarios;

  prepare_custom_scenarios(scenarios, cli);
  load_scenarios_from_files(scenarios, cli.scenario_files, cli);

  {
    std::vector<std::string> scenario_files;
    for (const auto &dir : cli.scenarios_dirs)
    {
      auto files = find_all_scenario_files(dir);
      scenario_files.insert(end(scenario_files), begin(files), end(files));
    }
    load_scenarios_from_files(scenarios, scenario_files, cli);
  }
  auto global_stats = run_scenarios(scenarios, cli);

  if (!cli.quiet)
  {
    print_stats(scenarios);
  }

  if (!cli.output_file.empty())
  {
    save_json(global_stats, cli.output_dir, cli.output_file);
  }

  return 0;
}
