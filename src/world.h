#pragma once

#include "load.h"
#include "types.h"

#include <memory>
#include <queue>
#include <random>

struct World {
  uint64_t seed_;
  Time time = 0;
  TimePeriod length = 10000;
  TimePeriod tick_length = 3;

  Uuid last_id = 0;

  using load_container_t = std::vector<Load>;
  std::priority_queue<Load, load_container_t, by_end_time> loads_served_{};
  std::priority_queue<Load, load_container_t, by_send_time> loads_send_{};
  std::mt19937_64 random_engine_{seed_};

  std::vector<std::unique_ptr<Group>> groups_{};

  Time advance();

  Uuid get_unique_id();

  std::mt19937_64 &get_random_engine();

  void add_group(std::unique_ptr<Group> group);
  void queue_load(Load load);
  bool serve_load(Load load);
  void check_loads();

  void print_stats();
};
