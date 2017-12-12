
#include "group.h"

#include <gsl/gsl>

Group::Group(World &world) : id(world.get_unique_id()), world_(world)
{
}

void Group::set_end_time(Load &load)
{
  load.end_time = load.send_time + gsl::narrow<Time>(load.size / serve_intensity);
}

void Group::add_load(Load load)
{
  served_load_size += load.size;
  load.served_by.reset(this);
  set_end_time(load);
  world_.queue_load(load);
}

void Group::serve(Load load)
{
  if (can_serve(load)) {
    add_load(load);
  } else {
    forward(load);
  }
}

bool Group::can_serve(const Load &load)
{
  return served_load_size + load.size < capacity;
}

void Group::forward(Load load)
{
  // TODO(PW): make it more intelligent
  if (!next_groups.empty()) {
    next_groups.front()->serve(load);
  } else {
    block_group->serve(load);
  }
}

void Group::take_off(const Load &load)
{
  served_load_size -= load.size;
}
