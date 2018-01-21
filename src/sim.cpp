
#include "sim.h"

#include "config.h"
#include "group.h"
#include "source_stream/source_stream.h"
#include "logger.h"
#include "topology_parser.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"

#include "scenarios/simple.h"
#include "scenarios/single_overflow.h"
#include "scenarios/topology_based.h"

#include <boost/program_options.hpp>
#include <experimental/memory>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <random>

constexpr Duration tick_length{0.5L};

uint64_t seed(bool use_random_seed)
{
  if (!use_random_seed) {
    return 0;
  }
  std::random_device rd;
  return rd();
}

void run_scenario(SimulationSettings &scenario,
                  const Duration duration,
                  bool use_random_seed,
                  bool quiet)
{
  scenario.world = std::make_unique<World>(seed(use_random_seed), duration, tick_length);
  auto &world = *scenario.world;
  world.set_topology(&scenario.topology);

  world.reset();
  world.init();
  if (scenario.do_before) {
    scenario.do_before();
  }
  world.run(quiet);
  if (scenario.do_after) {
    scenario.do_after();
  }
}


int main(int argc, char *argv[])
{
  setlocale(LC_NUMERIC, "en_US.UTF-8");

  namespace po = boost::program_options;

  /* clang-format off */
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("scenario-file,f", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
                        "a file with scenario")
    ("output-file,o", po::value<std::string>()->default_value(""),
                        "output file with stats")
    ("duration,t", po::value<time_type>()->default_value(100'000), "duration of the simulation")
    ("parallel,p", po::value<bool>()->default_value(true), "run simulations in parallel")
    ("start", po::value<intensity_t>()->default_value(0.5L), "starting intensity per group")
    ("stop", po::value<intensity_t>()->default_value(3.0L), "end intensity per group")
    ("step", po::value<intensity_t>()->default_value(0.5L), "step intensity per group")
    ("count,c", po::value<int>()->default_value(1), "number of repeats of each scenario")
    ("quiet,q", po::value<bool>()->default_value(false), "do not print stats")
    ("random,r",  "use random seed");
  /* clang-format on */

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help") > 0) {
    print("{}", desc);
    return 0;
  }

  const bool use_random_seed = vm.count("random") > 0;
  const bool quiet = vm.count("quiet") > 0;
  const std::string output_file = vm["output-file"].as<std::string>();
  const bool parallel{vm["parallel"].as<bool>()};
  const Duration duration{vm["duration"].as<time_type>()};
  const Intensity A_start{vm["start"].as<intensity_t>()};
  const Intensity A_stop{vm["stop"].as<intensity_t>() + 0.01L};
  const Intensity A_step{vm["step"].as<intensity_t>()};
  const int count = vm["count"].as<int>();

  const auto scenario_files = [&vm]() -> std::vector<std::string> {
    if (vm.count("scenario-file") > 0) {
      return vm["scenario-file"].as<std::vector<std::string>>();
    }
    return {};
  }();

  std::vector<std::string> args(argv + 1, argv + argc);
  std::vector<SimulationSettings> scenarios;

  if ((false)) {
    for (auto A = A_start; A <= A_stop; A += A_step) {
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

  /*
  for (auto A = Intensity{1.5L}; A <= Intensity{1.5L}; A += Intensity{0.1L}) {
    std::vector<Size> sizes{Size{1}};
    std::vector<int64_t> ratios{1};
    auto ratios_sum = std::accumulate(begin(ratios), end(ratios), 0);
    std::vector<long double> ratios_d(begin(ratios), end(ratios));
    for_each(begin(ratios_d), end(ratios_d),
             [ratios_sum](auto &x) { x /= ratios_sum; });

    const auto V = Capacity{30};
    const auto N = Count{20};

    std::vector<Intensity> intensities{sizes.size()};
    for (auto i = 0u; i < sizes.size(); ++i) {
      intensities[i] = Intensity{ts::get(A) * ts::get(V) / ts::get(sizes[i]) *
                                 ratios_d[i]};
    }
    scenarios.emplace_back(pascal_model(intensities[0], V, N));
  }
  */

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

  // scenarios.emplace_back(
  // single_overflow_engset(Intensity(1.0L), Capacity(4), Count(5)));

  // scenarios.emplace_back(multiple_sources_single_overflow());

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

  for (const auto &config_file : scenario_files) {
    const auto t = Config::parse_topology_config(config_file);
    // Config::dump(t);
    for (auto A = A_start; A <= A_stop; A += A_step) {
      for (int i = 0; i < count; ++i) {
        auto &scenario = scenarios.emplace_back(prepare_scenario_local_group_A(t, A));
        // auto &scenario = scenarios.emplace_back(prepare_scenario_global_A(t, A));
        scenario.name += fmt::format(" A={}", A);
        scenario.filename = config_file;
      }
    }
  }

  nlohmann::json global_stats = {};
  std::vector<bool> scenarios_state(scenarios.size());
  auto print_state = [](const std::vector<bool> &states) {
    constexpr auto width = 120;
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
      print("\033[2K\033[2F\033[2K"); // clear current and above line
    }
    // print("\033[H\033[J"); // clear ANSI terminal
    println("[Main]: Finished {}/{}: \n{}", finished, states.size(), str.str());
  };

#pragma omp parallel for if (parallel)
  for (auto i = 0ul; i < scenarios.size(); ++i) {
    run_scenario(scenarios[i], duration, use_random_seed, true);

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

  if (!quiet) {
    for (auto &scenario : scenarios) {
      print("\n[Main] {:-^100}\n", scenario.name);
      scenario.world->print_stats();

      if (scenario.do_after) {
        scenario.do_after();
      }
      print("[Main] {:^^100}\n", scenario.name);
    }
    {
      if (!output_file.empty()) {
        std::ofstream stats_file(output_file);
        stats_file << global_stats.dump(0);
      }
    }
  }

  return 0;
}
