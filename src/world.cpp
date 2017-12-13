
#include "world.h"

#include "group.h"
#include "load.h"
#include "logger.h"
#include "source_stream.h"

World::World(uint64_t seed, Duration duration, Duration tick_length)
  : seed_(seed), duration_(duration), tick_length_(tick_length)
{
}
void World::init()
{
  for (auto &source : sources_) {
    loads_send_.emplace(source->get(time_));
  }
}

bool World::next_iteration()
{
  debug_print("[World] Time = {}\n", time_);
  time_ += tick_length_;

  send_loads();
  serve_loads();
  return time_ <= duration_;
}

void World::queue_load_to_serve(Load load)
{
  loads_served_.emplace(load);
}

bool World::serve_load(Load load)
{
  if (!groups_.empty()) {
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
    loads_send_.emplace(load.produced_by->get(time_));
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

void World::add_group(std::unique_ptr<Group> group)
{
  groups_.emplace_back(std::move(group));
}

void World::add_source(std::unique_ptr<SourceStream> source)
{
  sources_.emplace_back(std::move(source));
}
void World::print_stats()
{
  print("[World] Left in queue {}\n", loads_served_.size());
  for (auto &group : groups_) {
    print("[World] Stats for {}: {}\n", *group, group->get_stats());
  }
}
