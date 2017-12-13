
#include "world.h"

#include "group.h"
#include "load.h"
#include "logger.h"
#include "source_stream.h"

World::World(uint64_t seed, Duration duration, Duration tick_length)
  : seed_(seed), duration_(duration), tick_length_(tick_length)
{
  print("[World] {:=^100}\n", " New world ");
}

World::~World()
{
  print("[World] {:^^100}\n", " End of the world ");
}
void World::init()
{
  for (auto &source : sources_) {
    loads_send_.emplace(source->get(time_));
  }
}

bool World::next_iteration()
{
  debug_print("[World] Time = {} "
              "----------------------------------------------------------------"
              "\n",
              time_);

  serve_loads();
  send_loads();

  Time next_event = 0;

  if (!loads_send_.empty()) {
    next_event = loads_send_.top().send_time;
  }
  if (!loads_served_.empty()) {
    next_event = std::min(next_event, loads_served_.top().end_time);
  }
  if (next_event > time_) {
    time_ = next_event;
  } else {
    print("No events\n");
    time_ += tick_length_;
  }

  return time_ <= duration_;
}

void World::queue_load_to_serve(Load load)
{
  loads_served_.emplace(load);
}

bool World::serve_load(Load load)
{
  if (load.target_group) {
    return load.target_group->serve(load);
  } else if (!groups_.empty()) {
    debug_print("[World] New load: {}\n", load);
    return groups_.front()->serve(load);
  }
  return false;
}

void World::send_loads()
{
  while (!loads_send_.empty() && loads_send_.top().send_time <= time_) {
    auto load = loads_send_.top();
    loads_send_.pop();
    loads_send_.emplace(load.produced_by->get(load.send_time));
    serve_load(load);
  }
}

void World::serve_loads()
{
  while (!loads_served_.empty() && loads_served_.top().end_time <= time_) {
    auto &load = loads_served_.top();
    load.served_by->take_off(load);
    loads_served_.pop();
  }
}

Uuid World::get_unique_id()
{
  return ++last_id;
}

std::mt19937_64 &World::get_random_engine()
{
  return random_engine_;
}

  void World::add_group(gsl::not_null<Group*> group)
{
  groups_.emplace_back(group);
}

  void World::add_source(gsl::not_null<SourceStream*> source)
{
  sources_.emplace_back(source);
}
void World::print_stats()
{
  print("[World] Left in queue {}\n", loads_served_.size());
  print("[World] Time = {:f}\n", time_);
  for (auto &group : groups_) {
    const auto &stats = group->get_stats();
    print("[World] Stats for {}: {}. Pblock {}\n", *group, stats,
          stats.block_time / duration_);
  }
}
