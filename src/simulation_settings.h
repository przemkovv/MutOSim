#pragma once

#include "types.h"

#include "topology.h"
#include <functional>

class World;

struct SimulationSettings {
  Name name;

  Topology topology{};

  std::function<void()> do_before = nullptr;
  std::function<void()> do_after = nullptr;

  std::unique_ptr<World> world = nullptr;
};
