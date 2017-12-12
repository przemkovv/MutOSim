
#include "world.h"

#include "group.h"
#include "load.h"
#include "logger.h"
#include "source_stream.h"

Time World::advance()
{
  debug_print("[World] Time = {}\n", time);
  time += tick_length;

  check_loads();
  return time;
}

void World::queue_load(Load load)
{
  loads_served_.emplace(std::move(load));
}

bool World::serve_load(Load load)
{
  if (!groups_.empty()) {
    debug_print("[World] New load: {}\n", load);
    return groups_.front()->serve(load);
  }
  return false;
}

void World::check_loads()
{
  while (!loads_send_.empty() && loads_send_.top().send_time <= time) {
    auto load = loads_send_.top();
    loads_send_.pop();
    loads_send_.emplace(load.produced_by->get(time));
    serve_load(load);
  }

  while (!loads_served_.empty() && loads_served_.top().end_time <= time) {
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

void World::print_stats()
{
  print("[World] Left in queue {}\n", loads_served_.size());
  for (auto &group : groups_) {
    print("[World] Stats for {}: {}\n", *group, group->get_stats());
  }
}
