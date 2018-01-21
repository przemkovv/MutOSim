#pragma once

#include "event.h"
#include "load.h"
#include "logger.h"
#include "stats.h"
#include "topology.h"
#include "types.h"

#include <experimental/memory>
#include <gsl/gsl>
#include <memory>
#include <queue>
#include <random>

#include <nlohmann/json.hpp>

struct Group;
class SourceStream;

class World
{
  using RandomEngine = std::mt19937_64;

  uint64_t seed_;
  Time time_{0};
  Duration duration_;
  Time finish_time_ = time_ + duration_;
  Duration tick_length_;

  Time current_time_{0}; // TODO(PW): find better name either for this or for time_ field

  Uuid last_id = 0;

  std::priority_queue<EventPtr, std::vector<EventPtr>, by_time> events_;
  RandomEngine random_engine_{seed_};

  observer_ptr<Topology> topology_;
  std::unordered_map<TrafficClassId, BlockStats> blocked_by_tc;
  std::unordered_map<Size, BlockStats> blocked_by_size;

  void process_event();

public:
  World(uint64_t seed, Duration duration, Duration tick_length);

  Uuid get_uuid();
  RandomEngine &get_random_engine();
  Duration get_tick_length() { return tick_length_; }
  Time get_time() const { return time_; }
  Time get_current_time() const { return current_time_; }
  auto get_progress() const { return Duration{time_} / duration_; }

  void set_topology(gsl::not_null<Topology *> topology);
  void schedule(std::unique_ptr<Event> event);

  void init();
  void reset();
  bool next_iteration();
  void run(bool quiet);

  void print_stats();
  nlohmann::json& append_stats(nlohmann::json &j);
  nlohmann::json get_stats();

  void update_block_stat(const Load &load);
  void update_unblock_stat(const Load &load);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const World &world);
