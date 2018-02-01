
#include "sim.h"

#include "group.h"
#include "logger.h"
#include "source_stream/source_stream.h"
#include "topology_parser.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"

#include "scenarios/simple.h"
#include "scenarios/single_overflow.h"
#include "scenarios/topology_based.h"

#include <boost/filesystem.hpp>
#include <boost/program_options/parsers.hpp>
#include <experimental/memory>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

void print_state(const std::vector<bool> &states)
{
  constexpr auto width = 120;
  const auto rows = states.size() / width + 2;
  std::stringstream str;
  int finished = 0;
  int current = 0;
  for (bool state : states) {
    str << (state ? "\033[48;2;255;255;0m " : "\033[48;2;155;155;155m ");
    finished += state;
    if (++current % width == 0)
      str << '\n';
  }
  str << "\033[0m";
  if (finished > 1) {
    for (auto row = 0u; row < rows; ++row) {
      print("\033[2K\033[1F"); // clear current and above line
    }
  }
  // print("\033[H\033[J"); // clear ANSI terminal
  println("[Main]: {} Finished {}/{}: \n{}", rows, finished, states.size(), str.str());
};

//----------------------------------------------------------------------

CLI parse_args(const boost::program_options::variables_map &vm)
{
  CLI cli;
  cli.help = vm.count("help") > 0;
  cli.use_random_seed = vm.count("random") > 0;
  cli.quiet = vm.count("quiet") > 0;
  cli.output_file = vm["output-file"].as<std::string>();
  cli.output_dir = vm["output-dir"].as<std::string>();
  cli.parallel = vm["parallel"].as<bool>();
  cli.duration = Duration{vm["duration"].as<time_type>()};
  cli.A_start = Intensity{vm["start"].as<intensity_t>()};
  cli.A_stop = Intensity{vm["stop"].as<intensity_t>() + 0.01L};
  cli.A_step = Intensity{vm["step"].as<intensity_t>()};
  cli.count = vm["count"].as<int>();

  cli.scenario_files = [&vm]() -> std::vector<std::string> {
    if (vm.count("scenario-file") > 0) {
      return vm["scenario-file"].as<std::vector<std::string>>();
    }
    return {};
  }();
  return cli;
}

//----------------------------------------------------------------------
boost::program_options::options_description prepare_options_description()
{
  namespace po = boost::program_options;
  /* clang-format off */
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("scenario-file,f", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
                        "a file with scenario")
    ("output-file,o", po::value<std::string>()->default_value(""),
                        "output file with stats")
    ("output-dir,d", po::value<std::string>()->default_value(""),
                        "output directory")
    ("duration,t", po::value<time_type>()->default_value(100'000), "duration of the simulation")
    ("parallel,p", po::value<bool>()->default_value(true), "run simulations in parallel")
    ("start", po::value<intensity_t>()->default_value(0.5L), "starting intensity per group")
    ("stop", po::value<intensity_t>()->default_value(3.0L), "end intensity per group")
    ("step", po::value<intensity_t>()->default_value(0.5L), "step intensity per group")
    ("count,c", po::value<int>()->default_value(1), "number of repeats of each scenario")
    ("quiet,q", po::value<bool>()->default_value(false), "do not print stats")
    ("random,r",  "use random seed");
  /* clang-format on */
  return desc;
}

//----------------------------------------------------------------------
nlohmann::json run_scenarios(std::vector<ScenarioSettings> &scenarios, const CLI &cli)
{
  nlohmann::json global_stats = {};
  std::vector<bool> scenarios_state(scenarios.size());

  sort(begin(scenarios), end(scenarios),
       [](const auto &s1, const auto &s2) { return s1.a > s2.a; });

#pragma omp parallel for schedule(guided, 8) if (cli.parallel)
  for (auto i = 0ul; i < scenarios.size(); ++i) {
    run_scenario(scenarios[i], cli.duration, cli.use_random_seed, true);

    auto A_str = std::to_string(ts::get(scenarios[i].A));
    auto filename = scenarios[i].filename;
#pragma omp critical
    {
      auto &scenario_stats = global_stats[filename][A_str];
      scenarios[i].world->append_stats(scenario_stats);
      scenario_stats["_a"] = ts::get(scenarios[i].a);
      scenario_stats["_A"] = ts::get(scenarios[i].A);

      scenarios_state[i] = true;
      print_state(scenarios_state);
    }
  }
  return global_stats;
}
//----------------------------------------------------------------------

void load_scenarios_from_files(std::vector<ScenarioSettings> &scenarios, const CLI &cli)
{
  for (const auto &config_file : cli.scenario_files) {
    const auto t = Config::parse_topology_config(config_file);
    // Config::dump(t);
    for (auto A = cli.A_start; A <= cli.A_stop; A += cli.A_step) {
      for (int i = 0; i < cli.count; ++i) {
        auto &scenario = scenarios.emplace_back(prepare_scenario_local_group_A(t, A));
        // auto &scenario = scenarios.emplace_back(prepare_scenario_global_A(t, A));
        scenario.name += fmt::format(" A={}", A);
        scenario.filename = config_file;
      }
    }
  }
}
//----------------------------------------------------------------------

void prepare_custom_scenarios(std::vector<ScenarioSettings> &scenarios, const CLI &cli)
{
  if ((false)) {
    for (auto A = cli.A_start; A <= cli.A_stop; A += cli.A_step) {
      std::vector<Size> sizes{Size{1}, Size{1}, Size{3}, Size{3}};
      std::vector<int64_t> ratios{1, 1, 1, 1};
      auto ratios_sum = std::accumulate(begin(ratios), end(ratios), 0ll);
      std::vector<long double> ratios_d(begin(ratios), end(ratios));
      for_each(begin(ratios_d), end(ratios_d),
               [ratios_sum](auto &x) { x /= ratios_sum; });

      auto V = Capacity{30};

      std::vector<Intensity> intensities{sizes.size()};
      for (auto i = 0u; i < sizes.size(); ++i) {
        intensities[i] =
            Intensity{ts::get(A) * ts::get(V) / ts::get(sizes[i]) * ratios_d[i]};
      }
      auto &scenario = scenarios.emplace_back(poisson_streams(intensities, sizes, V));
      scenario.name += fmt::format(" A={}", A);
    }
  }

  if ((false)) {
    scenarios.emplace_back(single_overflow_poisson(
        Intensity(24.0L), {Capacity{60}, Capacity{60}, Capacity{60}},
        {{Size{1}, Size{2}, Size{6}},
         {Size{1}, Size{2}, Size{6}},
         {Size{1}, Size{2}, Size{6}}},
        Capacity{42}));
  }

  if ((false)) {
    scenarios.emplace_back(single_overflow_poisson(Intensity(2.0L), Capacity(2)));
    scenarios.emplace_back(single_overflow_poisson(Intensity(4.0L), Capacity(2)));
    scenarios.emplace_back(single_overflow_poisson(Intensity(6.0L), Capacity(2)));
  }

  if ((false)) {
    scenarios.emplace_back(pascal_source_model(Intensity(1.0L), Capacity(1), Count(1)));
    scenarios.emplace_back(pascal_source_model(Intensity(1.0L), Capacity(2), Count(1)));
    scenarios.emplace_back(pascal_source_model(Intensity(1.0L), Capacity(1), Count(2)));
    scenarios.emplace_back(pascal_source_model(Intensity(1.0L), Capacity(1), Count(1)));
    scenarios.emplace_back(pascal_source_model(Intensity(1.0L), Capacity(2), Count(1)));
    scenarios.emplace_back(pascal_source_model(Intensity(1.0L), Capacity(3), Count(1)));
    scenarios.emplace_back(pascal_source_model(Intensity(1.0L), Capacity(4), Count(1)));
  }

  if ((false)) {
    scenarios.emplace_back(
        pascal_source_model(Intensity(15.0L), Capacity(30), Count(20)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(30.0L), Capacity(30), Count(20)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(45.0L), Capacity(30), Count(20)));
  }

  if ((false)) {
    scenarios.emplace_back(erlang_model(Intensity(15.0L), Capacity(30)));
    scenarios.emplace_back(erlang_model(Intensity(30.0L), Capacity(30)));
    scenarios.emplace_back(erlang_model(Intensity(45.0L), Capacity(30)));
  }

  if ((false)) {
    scenarios.emplace_back(engset_model(Intensity(10.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(engset2_model(Intensity(10.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(engset_model(Intensity(20.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(engset2_model(Intensity(20.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(engset_model(Intensity(30.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(engset2_model(Intensity(30.0L), Capacity(20), Count(40)));
  }
}
//----------------------------------------------------------------------
int main(int argc, char *argv[])
{
  setlocale(LC_NUMERIC, "en_US.UTF-8");

  namespace po = boost::program_options;

  auto desc = prepare_options_description();
  po::positional_options_description p;
  p.add("scenario-file", -1);

  po::variables_map vm;
  // po::store(po::parse_command_line(argc, argv, desc), vm);
  {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    std::ifstream cfg_file{"mutosim.cfg"};
    po::store(po::parse_config_file(cfg_file, desc), vm);
    po::notify(vm);
  }

  const CLI cli = parse_args(vm);

  if (cli.help) {
    print("{}", desc);
    return 0;
  }

  if (!std::all_of(begin(cli.scenario_files), end(cli.scenario_files),
                   [](const std::string &file) {
                     namespace fs = boost::filesystem;
                     if (auto path = fs::path{file}; exists(path)) {
                       return true;
                     } else {
                       println("[Main] Scenario file {} doesn't exists.", path);
                       return false;
                     }
                   })) {
    return ENOENT;
  }

  std::vector<ScenarioSettings> scenarios;

  prepare_custom_scenarios(scenarios, cli);
  load_scenarios_from_files(scenarios, cli);

  auto global_stats = run_scenarios(scenarios, cli);

  if (!cli.quiet) {
    for (auto &scenario : scenarios) {
      print("\n[Main] {:-^100}\n", scenario.name);
      scenario.world->print_stats();

      if (scenario.do_after) {
        scenario.do_after();
      }
      print("[Main] {:^^100}\n", scenario.name);
    }
  }

  if (!cli.output_file.empty()) {
    boost::filesystem::path output_file{cli.output_dir};
    output_file /= cli.output_file;
    create_directories(output_file.parent_path());
    std::ofstream stats_file(output_file.string());
    stats_file << global_stats.dump(0);
  }

  return 0;
}
