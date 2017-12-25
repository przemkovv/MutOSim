
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
constexpr Duration duration { 1'000'000};
// constexpr Duration duration{500'000};
// const auto duration = Duration(500'000);
// const auto duration = Duration(100'000);
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
  GroupName g1{"G1"};
  SourceName s1{"SPa1"};
  topology.add_group(std::make_unique<Group>(g1, V));

  topology.add_source(
      std::make_unique<PascalSourceStream>(s1, gamma, N, Size(1)));
  topology.attach_source_to_group(s1, g1);
  topology.add_traffic_class(s1, g1, serve_intensity);

  return sim_settings;
}

SimulationSettings multiple_sources_single_overflow()
{
  auto serve_intensity = Intensity(1.0L);
  const auto lambda = Intensity(3);
  const auto N = Count(2);
  const auto gamma = lambda / N;
  const auto V = Capacity(2);
  // const auto alpha = gamma / micro;

  SimulationSettings sim_settings{"Multiple sources - Single overflow"};

  auto &topology = sim_settings.topology;

  GroupName g1{"G1"};
  GroupName g2{"G2"};
  SourceName s1{"SPo1"};
  SourceName s2{"SEn2"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.add_group(std::make_unique<Group>(g2, V));
  topology.add_source(
      std::make_unique<PoissonSourceStream>(s1, lambda, Size(1)));
  topology.add_source(
      std::make_unique<EngsetSourceStream>(s2, gamma, N, Size(2)));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);
  topology.attach_source_to_group(s2, g1);

  topology.add_traffic_class(s1, g1, serve_intensity);
  topology.add_traffic_class(s1, g2, serve_intensity);
  topology.add_traffic_class(s2, g1, serve_intensity);
  topology.add_traffic_class(s2, g2, serve_intensity);

  return sim_settings;
}

int main()
{
  std::vector<SimulationSettings> scenarios;

  {
    scenarios.emplace_back(single_overflow_poisson(
        Intensity(24.0L), {Capacity{60}, Capacity{60}, Capacity{60}},
        {{Size{1}, Size{2}, Size{6}},
         {Size{1}, Size{2}, Size{6}},
         {Size{1}, Size{2}, Size{6}}},
        Capacity{42}));
    // scenarios.emplace_back(erlang_model(Intensity(3.0L), Capacity(1)));
    // scenarios.emplace_back(
    // engset_model(Intensity(1.0L), Capacity(1), Count(3)));

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

    if ((false)) {
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
