
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

#include <experimental/memory>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <memory>
#include <optional>
#include <random>

// constexpr auto duration = Duration(20'000'000);
// constexpr Duration duration { 5'000'000};
// constexpr Duration duration{1'000'000};
// constexpr Duration duration{500'000};
// const auto duration = Duration(500'000);
constexpr Duration duration = Duration(1'00'000);
// const auto duration = Duration(2000);
// const auto duration = Duration(500);
// const auto duration = Duration(100);
constexpr Duration tick_length{0.5L};

uint64_t seed()
{
  if constexpr (Config::constant_seed) {
    return 0;
  }
  std::random_device rd;
  return rd();
}

void run_scenario(SimulationSettings &scenario, bool quiet)
{
  scenario.world = std::make_unique<World>(seed(), duration, tick_length);
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

int main(int argc, char *argv[])
{
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() > 1) {
    auto t = Config::parse_topology_config(args[1]);
    Config::dump(t);
  }
  setlocale(LC_NUMERIC, "en_US.UTF-8");
  std::vector<SimulationSettings> scenarios;

  {
    if ((true))
      for (auto A = Intensity{0.5L}; A <= Intensity{1.51L}; A += Intensity{0.1L}) {
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

    // scenarios.emplace_back(single_overflow_poisson(
    // Intensity(24.0L), {Capacity{60}, Capacity{60}, Capacity{60}},
    // {{Size{1}, Size{2}, Size{6}},
    // {Size{1}, Size{2}, Size{6}},
    // {Size{1}, Size{2}, Size{6}}},
    // Capacity{42}));
    // scenarios.emplace_back(erlang_model(Intensity(3.0L), Capacity(1)));
    // scenarios.emplace_back(
    // engset_model(Intensity(1.0L), Capacity(1), Count(3)));

    // scenarios.emplace_back(single_overflow_poisson(
    // Intensity(24.0L), {Capacity{60}, Capacity{60}, Capacity{60}},
    // {{Size{1}, Size{2}, Size{6}},
    // {Size{1}, Size{2}, Size{6}},
    // {Size{1}, Size{2}, Size{6}}},
    // Capacity{42}));

    // scenarios.emplace_back(
    // single_overflow_poisson(Intensity(3.0L), Capacity(1)));
    // scenarios.emplace_back(
    // single_overflow_poisson(Intensity(3.0L), Capacity(1)));
    // scenarios.emplace_back(
    // single_overflow_poisson(Intensity(3.0L), Capacity(1)));

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

    if ((true)) {
      scenarios.emplace_back(erlang_model(Intensity(15.0L), Capacity(30)));
      scenarios.emplace_back(erlang_model(Intensity(30.0L), Capacity(30)));
      scenarios.emplace_back(erlang_model(Intensity(45.0L), Capacity(30)));
    }

    if ((true)) {
      scenarios.emplace_back(engset_model(Intensity(15.0L), Capacity(30), Count(20)));
      scenarios.emplace_back(engset_model(Intensity(30.0L), Capacity(30), Count(20)));
      scenarios.emplace_back(engset_model(Intensity(45.0L), Capacity(30), Count(20)));
    }

    if ((true)) {
#pragma omp parallel for
      for (auto i = 0ul; i < scenarios.size(); ++i) {
        auto &scenario = scenarios[i];
        run_scenario(scenario, true);
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
        run_scenario(scenario);
      }
    }
  }

  return 0;
}
