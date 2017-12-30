
#include "calculation.h"
#include "config.h"
#include "group.h"
#include "logger.h"
#include "sim.h"
#include "source_stream/engset.h"
#include "source_stream/pascal.h"
#include "source_stream/poisson.h"
#include "source_stream/source_stream.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"

#include "scenarios/simple.h"
#include "scenarios/single_overflow.h"

#include <nlohmann/json.hpp>

#include <experimental/memory>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <memory>
#include <optional>
#include <random>

// const auto duration = Duration(20'000'000);
// constexpr Duration duration { 5'000'000};
// constexpr Duration duration{1'000'000};
// constexpr Duration duration{500'000};
// const auto duration = Duration(500'000);
const auto duration = Duration(100'000);
// const auto duration = Duration(2000);
// const auto duration = Duration(100);
constexpr Duration tick_length{5};

uint64_t seed()
{
  if constexpr (Config::constant_seed) {
    return 0;
  }
  std::random_device rd;
  return rd();
}

SimulationSettings pascal_source_model(Intensity gamma, Capacity V, Count N)
{ // Pascal source

  auto serve_intensity = Intensity(1.0L);
  auto size = Size(1);

  auto name = fmt::format("Pascal source. gamma={}, V={}, N={} ", gamma, V, N);
  SimulationSettings sim_settings{name};

  // const auto lambda = Intensity(5);
  // const auto N = Size(5);
  // const auto gamma = lambda / N;
  // const auto micro = Intensity(1.0);
  // const auto V = Size(7);
  // const auto alpha = gamma / micro;

  sim_settings.do_before = [&]() {
    // print("[Engset] P_block = E(alfa, V, N) = {}\n", engset_pi(alpha, V, N,
    // V)); print("[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
    // engset_pi(alpha, V, N - 1, V));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;
  auto &tc = topology.add_traffic_class(gamma, serve_intensity, size);
  GroupName g1{"G1"};
  SourceName s1{"SPa1"};
  topology.add_group(std::make_unique<Group>(g1, V));

  topology.add_source(std::make_unique<PascalSourceStream>(s1, tc, N));
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}

SimulationSettings multiple_sources_single_overflow()
{
  auto serve_intensity = Intensity(1.0L);
  const auto lambda = Intensity(3);
  const auto N = Count(2);
  const auto gamma = lambda / N;
  const auto V = Capacity(2);
  const auto size1 = Size(1);
  const auto size2 = Size(2);
  // const auto alpha = gamma / micro;

  SimulationSettings sim_settings{"Multiple sources - Single overflow"};

  auto &topology = sim_settings.topology;

  auto &tc1 = topology.add_traffic_class(lambda, serve_intensity, size1);
  auto &tc2 = topology.add_traffic_class(gamma, serve_intensity, size2);

  GroupName g1{"G1"};
  GroupName g2{"G2"};
  SourceName s1{"SPo1"};
  SourceName s2{"SEn2"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.add_group(std::make_unique<Group>(g2, V));
  topology.add_source(std::make_unique<PoissonSourceStream>(s1, tc1));
  topology.add_source(std::make_unique<EngsetSourceStream>(s2, tc2, N));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);
  topology.attach_source_to_group(s2, g1);

  return sim_settings;
}

int main()
{
  setlocale(LC_NUMERIC, "en_US.UTF-8");
  std::vector<SimulationSettings> scenarios;

  {
    for (auto A = Intensity{0.5L}; A <= Intensity{1.5L}; A += Intensity{0.1L}) {
      std::vector<Size> sizes{Size{1}, Size{1}, Size{3}, Size{3}};
      std::vector<int64_t> ratios{1, 1, 1, 1};
      auto ratios_sum = std::accumulate(begin(ratios), end(ratios), 0);
      std::vector<long double> ratios_d(begin(ratios), end(ratios));
      for_each(begin(ratios_d), end(ratios_d),
               [ratios_sum](auto &x) { x /= ratios_sum; });

      auto V = Capacity{50};

      std::vector<Intensity> intensities{sizes.size()};
      for (auto i = 0u; i < sizes.size(); ++i) {
        intensities[i] = Intensity{ts::get(A) * ts::get(V) / ts::get(sizes[i]) *
                                   ratios_d[i]};
      }
      scenarios.emplace_back(poisson_streams(intensities, sizes, Capacity{50}));
    }

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
    scenarios.emplace_back(erlang_model(Intensity(3.0L), Capacity(1)));
    scenarios.emplace_back(
        engset_model(Intensity(1.0L), Capacity(1), Count(3)));

    // scenarios.emplace_back(
    // single_overflow_poisson(Intensity(3.0L), Capacity(1)));
    // scenarios.emplace_back(
    // single_overflow_poisson(Intensity(3.0L), Capacity(1)));
    // scenarios.emplace_back(
    // single_overflow_poisson(Intensity(3.0L), Capacity(1)));

    // scenarios.emplace_back(
    // single_overflow_engset(Intensity(1.0L), Capacity(1), Count(3)));

    // scenarios.emplace_back(multiple_sources_single_overflow());

    // scenarios.emplace_back(
    // pascal_source_model(Intensity(1), Capacity(10), Count(1)));
    // scenarios.emplace_back(
    // pascal_source_model(Intensity(1), Capacity(10), Count(5)));
    // scenarios.emplace_back(
    // pascal_source_model(Intensity(1), Capacity(10), Count(10)));

    auto run_scenario = [&](auto &scenario, bool quiet = false) {
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
    };

    if ((true)) {
#pragma omp parallel for
      for (auto i = 0ul; i < scenarios.size(); ++i) {
        auto &scenario = scenarios[i];
        run_scenario(scenario, true);
      }
      for (auto &scenario : scenarios) {
        print("\n[Main] {:-^100}\n", scenario.name);
        scenario.world->print_stats();
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
