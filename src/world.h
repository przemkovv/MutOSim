#pragma once

#include "load.h"
#include "types.h"

#include <gsl/gsl>
#include <memory>
#include <queue>
#include <random>

struct Group;
class SourceStream;

class World
{
  using RandomEngine = std::mt19937_64;

  uint64_t seed_;
  Time time_ = 0;
  Duration duration_;
  Duration tick_length_;

  Uuid last_id = 0;

  std::priority_queue<Load, std::vector<Load>, by_end_time> loads_served_{};
  std::priority_queue<Load, std::vector<Load>, by_send_time> loads_send_{};
  RandomEngine random_engine_{seed_};

  std::vector<Group *> groups_{};
  std::vector<SourceStream *> sources_{};

  bool serve_load(Load load);

  void send_loads();
  void serve_loads();

public:
  World(uint64_t seed, Duration duration, Duration tick_length);
  ~World();

  Uuid get_unique_id();
  RandomEngine &get_random_engine();
  Duration get_tick_length() { return tick_length_; }
  Duration get_time() { return time_; }
  auto get_progress() { return time_ / duration_; }

  void add_group(gsl::not_null<Group *> group);
  void add_source(gsl::not_null<SourceStream *> source);

  void queue_load_to_serve(Load load);

  void init();
  bool next_iteration();

  void print_stats();
};
