#pragma once

#include "event.h"
#include "load.h"
#include "logger.h"
#include "topology.h"
#include "types.h"

#include <experimental/memory>
#include <gsl/gsl>
#include <memory>
#include <queue>
#include <random>

struct Group;
class SourceStream;

using std::experimental::make_observer;
using std::experimental::observer_ptr;

class World
{
  using RandomEngine = std::mt19937_64;

  uint64_t seed_;
  Time time_ = 0;
  Duration duration_;
  Duration tick_length_;

  Uuid last_id = 0;

  std::priority_queue<EventPtr, std::vector<EventPtr>, by_time> events_;
  RandomEngine random_engine_{seed_};

  observer_ptr<Topology> topology_;

  void process_event();

public:
  World(uint64_t seed, Duration duration, Duration tick_length);
  ~World();

  Uuid get_uuid();
  RandomEngine &get_random_engine();
  Duration get_tick_length() { return tick_length_; }
  Time get_time() { return time_; }
  auto get_progress() { return time_ / duration_; }

  void set_topology(gsl::not_null<Topology *> topology);
  void schedule(std::unique_ptr<Event> event);

  void init();
  bool next_iteration();
  void run(bool quiet);

  void print_stats();
};
