
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
  for (const auto &[name, group] : config.groups) {
    topology.add_group(std::make_unique<Group>(name, group.capacity));
    V += group.capacity;
  }
  for (const auto &[name, group] : config.groups) {
    for (const auto &connected_group : group.connected) {
      topology.connect_groups(name, connected_group);
    }
  }
  Weight sum = std::accumulate(
      begin(config.traffic_classes), end(config.traffic_classes), Weight{0},
      [](const auto x, const auto &tc) { return tc.second.weight + x; });

  for (const auto &[tc_id, tc] : config.traffic_classes) {
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

std::unique_ptr<overflow_policy::OverflowPolicy>
make_overflow_policy(std::optional<std::string_view> name, gsl::not_null<Group *> group)
{
  using namespace overflow_policy;
  using namespace std::literals;
  if (name) {
    if (name == "random_available"sv) {
      return std::make_unique<RandomAvailable>(group);
    }
    if (name == "first_available"sv) {
      return std::make_unique<AlwaysFirst>(group);
    }
    if (name == "always_first"sv) {
      return std::make_unique<AlwaysFirst>(group);
    }
    if (name == "no_overflow"sv) {
      return std::make_unique<NoOverflow>(group);
    }
    if (name == "default"sv) {
      return std::make_unique<Default>(group);
    }
    print("[Main]: Don't recognize '{}' overflow policy name. Using default.", *name);
  }
  return std::make_unique<Default>(group);
}

SimulationSettings prepare_scenario_local_group_A(const Config::Topology &config,
                                                  Intensity A)
{
  SimulationSettings sim_settings{config.name};
  Capacity V{0};

  auto &topology = sim_settings.topology;
  for (const auto &[name, group] : config.groups) {
    auto &g =
        topology.add_group(std::make_unique<Group>(name, group.capacity, group.layer));
    g.set_overflow_policy(make_overflow_policy(group.overflow_policy, &g));
    V += group.capacity;
  }
  for (const auto &[name, group] : config.groups) {
    for (const auto &connected_group : group.connected) {
      topology.connect_groups(name, connected_group);
    }
  }

  std::unordered_map<GroupName, Weight> weights_sum_per_group;
  for (const auto &source : config.sources) {
    weights_sum_per_group[source.attached] +=
        config.traffic_classes.at(source.tc_id).weight;
  }

  Intensity traffic_intensity{0};
  for (const auto &source : config.sources) {
    const auto &cfg_tc = config.traffic_classes.at(source.tc_id);
    const auto ratio = cfg_tc.weight / weights_sum_per_group[source.attached];
    const auto &group = topology.get_group(source.attached);
    const auto intensity_multiplier =
        config.groups.at(group.get_name()).intensity_multiplier;
    Intensity offered_intensity =
        A * intensity_multiplier * group.capacity_ * ratio / cfg_tc.size;
    const auto &tc =
        topology.add_traffic_class(cfg_tc.id, offered_intensity, cfg_tc.serve_intensity,
                                   cfg_tc.size, cfg_tc.max_path_length);

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
    ("count,c", po::value<int>()->default_value(1), "number of repeats of each scenario")
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
  {
    // print("Time type precision (digits): {}\n",
    // std::numeric_limits<time_type>::digits10);
  }

  std::vector<std::string> args(argv + 1, argv + argc);
  std::vector<SimulationSettings> scenarios;

  {
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
      std::string s;
      constexpr auto width = 120;
      s.resize(states.size() + states.size()/width);
      std::stringstream str;
      int finished =0;
      int current = 0;
      for (bool state : states) {
        str << (state ? "\033[48;2;255;255;0m " : "\033[48;2;155;155;155m ");
        finished += state;
        if (++current % width == 0) str << '\n';
      }
      str << "\033[0m";
      print("\033[H\033[J"); // clear ANSI terminal
      println("[Main]: Finished {}/{}: \n{}", finished, states.size(), str.str());
    };
    if ((parallel)) {
#pragma omp parallel for
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

        auto A_str = std::to_string(ts::get(scenario.A));
        auto filename = scenario.filename;
        auto &scenario_stats = global_stats[filename][A_str];
        scenario.world->append_stats(scenario_stats);
        scenario_stats["_a"] = ts::get(scenario.a);
        scenario_stats["_A"] = ts::get(scenario.A);

        scenario.world->print_stats();
        if (scenario.do_after) {
          scenario.do_after();
        }
        print("[Main] {:^^100}\n", scenario.name);
      }
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
