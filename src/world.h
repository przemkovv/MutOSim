#pragma once

#include "load.h"
#include "types.h"

#include <queue>
#include <random>

struct Load;

struct World {
  uint64_t seed_;
  Time time = 0;
  TimePeriod length = 10000;
  TimePeriod tick_length = 3;

  Uuid last_id = 0;

  std::priority_queue<Load> loads{};
  std::mt19937_64 random_engine_{seed_};

  Time advance()
  {
    time += tick_length;
    return time;
  }

  void queue_load(Load load) { loads.emplace(std::move(load)); }

  Uuid get_unique_id() { return ++last_id; }

  auto &get_random_engine() { return random_engine_; }
};
