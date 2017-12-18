
#include "calculation.h"
#include "config.h"
#include "group.h"
#include "logger.h"
#include "sim.h"
#include "source_stream/engset.h"
#include "source_stream/poisson.h"
#include "source_stream/source_stream.h"
#include "types.h"
#include "world.h"

#include <experimental/memory>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <memory>
#include <optional>
#include <parallel/algorithm>
#include <random>

using std::experimental::make_observer;

// const auto duration = Duration(20'000'000);
const auto duration = Duration(1'000'000);
// const auto duration = Duration(500'000);
// const auto duration = Duration(100'000);
// const auto duration = Duration(2000);
const auto tick_length = Duration(0.1);

uint64_t seed()
{
  if constexpr (Config::constant_seed) {
    return 0;
  }
  std::random_device rd;
  return rd();
}

SimulationSettings erlang_model()
{
  SimulationSettings sim_settings{duration, tick_length, "Erlang model"};

  const auto lambda = Intensity(3.0);
  const auto micro = Intensity(1.0);
  const auto V = Size(1);
  const auto A = lambda / micro;
  sim_settings.do_before = [&]() {
    print("[Erlang] P_loss = P_block = E_V(A) = {}\n", erlang_pk(A, V, V));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;
  topology.add_group(std::make_unique<Group>("G1", V, micro));

  topology.add_source(
      std::make_unique<PoissonSourceStream>("S1", lambda, Size(1)));
  topology.attach_source_to_group("S1", "G1");

  return sim_settings;
}

SimulationSettings engset_model()
{ // Engset model

  SimulationSettings sim_settings{duration, tick_length, "Engset model"};

  const auto lambda = Intensity(3);
  const auto N = Size(2);
  const auto gamma = lambda / N;
  const auto micro = Intensity(1.0);
  const auto V = Size(1);
  const auto alpha = gamma / micro;

  sim_settings.do_before = [&]() {
    print("[Engset] P_block = E(alfa, V, N) = {}\n", engset_pi(alpha, V, N, V));
    print("[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
          engset_pi(alpha, V, N - 1, V));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;
  topology.add_group(std::make_unique<Group>("G1", V, micro));

  topology.add_source(
      std::make_unique<EngsetSourceStream>("S1", gamma, N, Size(1)));
  topology.attach_source_to_group("S1", "G1");

  return sim_settings;
}

SimulationSettings single_overflow()
{
  SimulationSettings sim_settings{duration, tick_length, "Single overflow"};

  auto &topology = sim_settings.topology;
  topology.add_group(std::make_unique<Group>("G1", Size(1), Intensity(1.0)));
  topology.add_group(std::make_unique<Group>("G2", Size(1), Intensity(1.0)));
  topology.add_source(
      std::make_unique<PoissonSourceStream>("S1", Intensity(3.0), Size(1)));

  topology.connect_groups("G1", "G2");
  topology.attach_source_to_group("S1", "G1");

  return sim_settings;
}

SimulationSettings multiple_sources_single_overflow()
{
  const auto lambda = Intensity(3);
  const auto N = Size(2);
  const auto gamma = lambda / N;
  const auto micro = Intensity(1.0);
  const auto V = Size(1);
  // const auto alpha = gamma / micro;

  SimulationSettings sim_settings{duration, tick_length,
                                  "Multiple sources - Single overflow"};

  auto &topology = sim_settings.topology;
  topology.add_group(std::make_unique<Group>("G1", V, micro));
  topology.add_group(std::make_unique<Group>("G2", V, micro));
  topology.add_source(
      std::make_unique<PoissonSourceStream>("SP1", lambda, Size(1)));
  topology.add_source(
      std::make_unique<EngsetSourceStream>("SE2", gamma, N, Size(1)));

  topology.connect_groups("G1", "G2");
  topology.attach_source_to_group("SP1", "G1");
  topology.attach_source_to_group("SE2", "G1");

  return sim_settings;
}

int main()
{
  std::vector<SimulationSettings> scenarios;

  {
    scenarios.emplace_back(erlang_model());
    scenarios.emplace_back(engset_model());
    scenarios.emplace_back(single_overflow());
    scenarios.emplace_back(multiple_sources_single_overflow());

    auto run_scenario = [](auto &scenario) {
      print("[Main] {:-^100}\n", scenario.name);
      World world{seed(), scenario.duration, scenario.tick_length};
      world.set_topology(&scenario.topology);

      world.init();
      if (scenario.do_before) {
        scenario.do_before();
      }
      world.run();
      if (scenario.do_after) {
        scenario.do_after();
      }
    };

    if ((false)) {
#pragma omp parallel for
      for (auto i = 0ul; i < scenarios.size(); ++i) {
        auto &scenario = scenarios[i];
        run_scenario(scenario);
      }
    } else {
      for (auto &scenario : scenarios) {
        run_scenario(scenario);
      }
    }
  }

  return 0;
}
