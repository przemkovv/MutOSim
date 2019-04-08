#pragma once

#include "cli_options.h"
#include "topology.h"
#include "types/types.h"

#include <boost/container/flat_map.hpp>
#include <functional>
#include <nlohmann/json.hpp>

namespace Simulation {
class World;
}

struct ScenarioSettings
{
  Name                  name;
  Simulation::Intensity A{0};
  Simulation::Intensity a{0};

  std::string filename = "";

  Mode                                                mode{Mode::Analytic};
  Model::AnalyticModel                                analytic_model{};
  boost::container::flat_map<Layer, Model::LayerType> layers_types{};

  Simulation::Topology topology{};
  nlohmann::json       json{};
  nlohmann::json       stats{};

  std::function<void()> do_before = nullptr;
  std::function<void()> do_after = nullptr;

  std::unique_ptr<Simulation::World> world{};
};

uint64_t seed(bool use_random_seed);
void
run_scenario(ScenarioSettings &scenario, const Duration duration, bool use_random_seed, bool quiet);
