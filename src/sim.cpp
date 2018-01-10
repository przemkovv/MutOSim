
#include "calculation.h"
#include "config.h"
#include "group.h"
#include "logger.h"
#include "sim.h"
#include "source_stream/engset.h"
#include "source_stream/pascal.h"
#include "source_stream/poisson.h"
#include "source_stream/source_stream.h"
#include "topology_parser.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"

#include "scenarios/simple.h"
#include "scenarios/single_overflow.h"

#include <boost/program_options.hpp>
#include <experimental/memory>
#include <fmt/format.h>
#include <fmt/printf.h>
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

  world.init();
  if (scenario.do_before) {
    scenario.do_before();
  }
  world.run(quiet);
  if (scenario.do_after) {
    scenario.do_after();
  }
}

std::unique_ptr<SourceStream> create_stream(Config::SourceType type,
                                            const Config::Source &source,
                                            const TrafficClass &tc)
{
  switch (type) {
  case Config::SourceType::Poisson:
    return std::make_unique<PoissonSourceStream>(source.name, tc);
  case Config::SourceType::Pascal:
    return std::make_unique<PascalSourceStream>(source.name, tc, source.source_number);
  case Config::SourceType::Engset:
    return std::make_unique<EngsetSourceStream>(source.name, tc, source.source_number);
  }
}

SimulationSettings prepare_scenario_global_A(const Config::Topology &config, Intensity A)
{
  SimulationSettings sim_settings{config.name};
  Capacity V{0};

  auto &topology = sim_settings.topology;
  for (const auto &group : config.groups) {
    topology.add_group(std::make_unique<Group>(group.name, group.capacity));
    V += group.capacity;
  }
  for (const auto &group : config.groups) {
    for (const auto &connected_group : group.connected) {
      topology.connect_groups(group.name, connected_group);
    }
  }
  Weight sum = std::accumulate(
      begin(config.traffic_classes), end(config.traffic_classes), Weight{0},
      [](const auto x, const auto &tc) { return tc.weight + x; });

  for (const auto &tc : config.traffic_classes) {
    const auto ratio = tc.weight / sum;
    Intensity offered_intensity = A * V * ratio / tc.size;
    topology.add_traffic_class(tc.id, offered_intensity, tc.serve_intensity, tc.size);
  }

  for (const auto &source : config.sources) {
    auto &tc = topology.get_traffic_class(source.tc_id);
    topology.add_source(create_stream(source.type, source, tc));
    topology.attach_source_to_group(source.name, source.attached);
  }

  return sim_settings;
}

SimulationSettings prepare_scenario_local_group_A(const Config::Topology &config,
                                                  Intensity A)
{
  SimulationSettings sim_settings{config.name};
  Capacity V{0};

  auto &topology = sim_settings.topology;
  for (const auto &group : config.groups) {
    topology.add_group(std::make_unique<Group>(group.name, group.capacity));
    V += group.capacity;
  }
  for (const auto &group : config.groups) {
    for (const auto &connected_group : group.connected) {
      topology.connect_groups(group.name, connected_group);
    }
  }

  std::unordered_map<GroupName, Weight> weights_sum_per_group;
  for (const auto &source : config.sources) {
    weights_sum_per_group[source.attached] +=
        config.traffic_classes[ts::get(source.tc_id)].weight;
  }

  Intensity traffic_intensity{0};
  for (const auto &source : config.sources) {
    const auto &cfg_tc = config.traffic_classes[ts::get(source.tc_id)];
    const auto ratio = cfg_tc.weight / weights_sum_per_group[source.attached];
    const auto &group = topology.get_group(source.attached);
    Intensity offered_intensity = A * group.capacity_ * ratio / cfg_tc.size;
    auto &tc = topology.add_traffic_class(cfg_tc.id, offered_intensity,
                                          cfg_tc.serve_intensity, cfg_tc.size);

    topology.add_source(create_stream(source.type, source, tc));
    topology.attach_source_to_group(source.name, source.attached);

    traffic_intensity += Intensity{tc.source_intensity / tc.serve_intensity *
                                   ts::get(tc.size) / ts::get(V)};
  }
  // traffic_intensity /= V;

  sim_settings.name += fmt::format(" a={}", traffic_intensity);
  sim_settings.a = traffic_intensity;
  sim_settings.A = A;
  return sim_settings;
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
    ("random,r",  "use random seed");
  /* clang-format on */

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    print("{}", desc);
    return 0;
  }

  const bool use_random_seed = vm.count("random") > 0;
  const std::string output_file = vm["output-file"].as<std::string>();
  const bool parallel{vm["parallel"].as<bool>()};
  const Duration duration{vm["duration"].as<time_type>()};
  const Intensity A_start{vm["start"].as<intensity_t>()};
  const Intensity A_stop{vm["stop"].as<intensity_t>() + 0.01L};
  const Intensity A_step{vm["step"].as<intensity_t>()};

  const auto scenario_files = [&vm]() -> std::vector<std::string> {
    if (vm.count("scenario-file"))
      return vm["scenario-file"].as<std::vector<std::string>>();
    else
      return {};
  }();
  {
    // print("Time type precision (digits): {}\n",
    // std::numeric_limits<time_type>::digits10);
  }

  std::vector<std::string> args(argv + 1, argv + argc);
  std::vector<SimulationSettings> scenarios;

  {
    if ((false))
      for (auto A = A_start; A <= A_stop; A += A_step) {
        std::vector<Size> sizes{Size{1}, Size{1}, Size{3}, Size{3}};
        std::vector<int64_t> ratios{1, 1, 1, 1};
        auto ratios_sum = std::accumulate(begin(ratios), end(ratios), 0);
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
        auto &scenario = scenarios.emplace_back(prepare_scenario_local_group_A(t, A));
        // auto &scenario = scenarios.emplace_back(prepare_scenario_global_A(t, A));
        scenario.name += fmt::format(" A={}", A);
        scenario.filename = config_file;
      }
    }

    if ((parallel)) {
#pragma omp parallel for
      for (auto i = 0ul; i < scenarios.size(); ++i) {
        auto &scenario = scenarios[i];
        run_scenario(scenario, duration, use_random_seed, true);
      }

      for (auto &scenario : scenarios) {
        print("\n[Main] {:-^100}\n", scenario.name);
        scenario.world->print_stats();

        if (scenario.do_after) {
          scenario.do_after();
        }
        print("[Main] {:^^100}\n", scenario.name);
      }
    } else {
      for (auto &scenario : scenarios) {
        print("\n[Main] {:-^100}\n", scenario.name);
        run_scenario(scenario, duration, use_random_seed, true);

        scenario.world->print_stats();
        if (scenario.do_after) {
          scenario.do_after();
        }
        print("[Main] {:^^100}\n", scenario.name);
      }
    }

    {
      nlohmann::json sims_stats = {};
      for (auto &scenario : scenarios) {
        auto scenario_stats = scenario.world->get_stats();
        scenario_stats["_a"] = ts::get(scenario.a);
        scenario_stats["_A"] = ts::get(scenario.A);
        scenario_stats["_name"] = scenario.name;
        scenario_stats["_scenario_file"] = scenario.filename;

        sims_stats[std::to_string(ts::get(scenario.A))] = scenario_stats;
      }
      if (!output_file.empty()) {
        std::ofstream stats_file(output_file);
        stats_file << sims_stats.dump(2);
      }
    }
  }

  return 0;
}
