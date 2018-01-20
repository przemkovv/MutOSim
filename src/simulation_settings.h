#pragma once

#include "types.h"

#include "topology.h"
#include <functional>
#include <nlohmann/json.hpp>

class World;

struct SimulationSettings {
  Name name;
  Intensity A{0};
  Intensity a{0};

  std::string filename = "";

  Topology topology{};

  std::function<void()> do_before = nullptr;
  std::function<void()> do_after = nullptr;

  std::unique_ptr<World> world = nullptr;
  nlohmann::json stats{};
};
