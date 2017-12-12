#pragma once

#include "types.h"

#include <memory>
#include <queue>
#include <random>

struct Load;
struct Group;

struct World {
  uint64_t seed_;
  Time time = 0;
  TimePeriod length = 10000;
  TimePeriod tick_length = 3;

  Uuid last_id = 0;

  std::priority_queue<Load> loads{};
  std::mt19937_64 random_engine_{seed_};

  std::vector<std::unique_ptr<Group>> groups_{};

  Time advance();

  Uuid get_unique_id();

  std::mt19937_64 &get_random_engine();

  void add_group(std::unique_ptr<Group> group);
  void queue_load(Load load);
  void serve_load(Load load);
  void check_loads();
};
