#pragma once

#include "topology.h"
#include "types.h"

#include <functional>
#include <nlohmann/json.hpp>

namespace Simulation
{
class World;
}

struct ScenarioSettings {
  Name name;
  Intensity A{0};
  Intensity a{0};

  std::string filename = "";

  Simulation::Topology topology{};
  nlohmann::json json{};

  std::function<void()> do_before = nullptr;
  std::function<void()> do_after = nullptr;

  std::unique_ptr<Simulation::World> world{};
};

uint64_t seed(bool use_random_seed);
void run_scenario(
    ScenarioSettings &scenario,
    const Duration duration,
    bool use_random_seed,
    bool quiet);
