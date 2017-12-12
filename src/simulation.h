#pragma once

#include "types.h"
#include "load.h"

#include <queue>

struct Load;

struct World {
  Time time = 0;
  TimePeriod length = 10000;
  TimePeriod tick_length = 3;

  Uuid last_id = 0;

  std::priority_queue<Load> loads{};

  Time advance()
  {
    time += tick_length;
    return time;
  }

  void queue_load(Load load) { loads.emplace(std::move(load)); }

  Uuid get_unique_id() {
    return ++last_id;
  }
};
