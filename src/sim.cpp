
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

#include "nlohmann/json.hpp"

#include <experimental/memory>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <memory>
#include <optional>
#include <random>

// const auto duration = Duration(20'000'000);
// constexpr auto duration = 5'000'000;
constexpr Duration duration{500'000};
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

SimulationSettings erlang_model(const Intensity lambda, const Size V)
{
  TrafficClass tc1;
  tc1.source_intensity = lambda;
  tc1.serve_intensity = Intensity(1.0);
  tc1.size = Size(1);

  SimulationSettings sim_settings{duration, tick_length, "Erlang model"};

  const auto micro = tc1.serve_intensity;
  const auto A = lambda / micro;
  sim_settings.do_before = [=]() {
    print("[Erlang] P_loss = P_block = E_V(A) = {}\n", erlang_pk(A, V, V));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;

  tc1.source_id = topology
                      .add_source(std::make_unique<PoissonSourceStream>(
                          SourceName("SPo1"), lambda, tc1.size))
                      .id;

  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Group>(g1, V)).add_traffic_class(tc1);
  topology.attach_source_to_group(SourceName("SPo1"), g1);

  return sim_settings;
}

SimulationSettings
engset_model(const Intensity gamma, const Size V, const Size N)
{ // Engset model

  SimulationSettings sim_settings{duration, tick_length, "Engset model"};

  const auto micro = Intensity(1.0);
  const auto alpha = gamma / micro;

  sim_settings.do_before = [=]() {
    print("[Engset] P_block = E(alfa, V, N) = {}\n", engset_pi(alpha, V, N, V));
    print("[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
          engset_pi(alpha, V, N - 1, V));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;
  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Group>(g1, V));

  topology.add_source(std::make_unique<EngsetSourceStream>(SourceName("SEn1"),
                                                           gamma, N, Size(1)));
  topology.attach_source_to_group(SourceName("SEn1"), g1);

  return sim_settings;
}
SimulationSettings pascal_source_model(Intensity gamma, Size V, Size N)
{ // Pascal source

  auto name = fmt::format("Pascal source. gamma={}, V={}, N={} ", gamma, V, N);
  SimulationSettings sim_settings{duration, tick_length, name};

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
  topology.add_group(std::make_unique<Group>(g1, V));

  topology.add_source(std::make_unique<PascalSourceStream>(SourceName("SPa1"),
                                                           gamma, N, Size(1)));
  topology.attach_source_to_group(SourceName("SPa1"), g1);

  return sim_settings;
}

SimulationSettings single_overflow()
{
  SimulationSettings sim_settings{duration, tick_length, "Single overflow"};

  auto &topology = sim_settings.topology;
  GroupName g1{"G1"};
  GroupName g2{"G2"};
  topology.add_group(std::make_unique<Group>(g1, Size(1)));
  topology.add_group(std::make_unique<Group>(g2, Size(1)));
  topology.add_source(std::make_unique<PoissonSourceStream>(
      SourceName("SPo1"), Intensity(3.0), Size(1)));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(SourceName("SPo1"), g1);

  return sim_settings;
}

SimulationSettings multiple_sources_single_overflow()
{
  const auto lambda = Intensity(3);
  const auto N = Size(2);
  const auto gamma = lambda / N;
  // const auto micro = Intensity(1.0);
  const auto V = Size(1);
  // const auto alpha = gamma / micro;

  SimulationSettings sim_settings{duration, tick_length,
                                  "Multiple sources - Single overflow"};

  auto &topology = sim_settings.topology;

  GroupName g1{"G1"};
  GroupName g2{"G2"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.add_group(std::make_unique<Group>(g2, V));
  topology.add_source(std::make_unique<PoissonSourceStream>(SourceName("SPo1"),
                                                            lambda, Size(1)));
  topology.add_source(std::make_unique<EngsetSourceStream>(SourceName("SEn2"),
                                                           gamma, N, Size(1)));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(SourceName("SPo1"), g1);
  topology.attach_source_to_group(SourceName("SEn2"), g1);

  return sim_settings;
}

int main()
{
  std::vector<SimulationSettings> scenarios;

  {
    scenarios.emplace_back(erlang_model(Intensity(3.0), Size(1)));
    // scenarios.emplace_back(engset_model(Intensity(1.0), Size(1), Size(3)));
    // scenarios.emplace_back(single_overflow());
    // scenarios.emplace_back(multiple_sources_single_overflow());
    // scenarios.emplace_back(
    // pascal_source_model(Intensity(1), Size(10), Size(1)));
    // scenarios.emplace_back(
    // pascal_source_model(Intensity(1), Size(10), Size(5)));
    // scenarios.emplace_back(
    // pascal_source_model(Intensity(1), Size(10), Size(10)));

    auto run_scenario = [](auto &scenario, bool quiet = false) {
      scenario.world = std::make_unique<World>(seed(), scenario.duration,
                                               scenario.tick_length);
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
        print("[Main] {:-^100}\n", scenario.name);
        scenario.world->print_stats();
        print("[Main] {:^^100}\n", scenario.name);
      }
    } else {
      for (auto &scenario : scenarios) {
        print("[Main] {:-^100}\n", scenario.name);
        run_scenario(scenario);
      }
    }
  }

  return 0;
}
