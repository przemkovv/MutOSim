
#include "world.h"

#include "group.h"
#include "load.h"

Time World::advance()
{
  time += tick_length;

  check_loads();
  return time;
}

void World::queue_load(Load load)
{
  loads.emplace(std::move(load));
}

void World::serve_load(Load load)
{
  if (!groups_.empty()) {
    groups_.front()->serve(load);
  }
}

void World::check_loads()
{
  while (!loads.empty() && loads.top().end_time < time) {
    auto &load = loads.top();
    load.served_by->take_off(load);
    loads.pop();
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
