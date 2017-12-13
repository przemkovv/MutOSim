
#include "group.h"
#include "logger.h"

#include <cmath>
#include <gsl/gsl>

Group::Group(World &world, Size capacity, Intensity serve_intensity)
  : id(world.get_unique_id()),
    capacity_(capacity),
    serve_intensity_(serve_intensity),
    world_(world),
    loss_group(world)
{
}

void Group::set_end_time(Load &load)
{
  // auto pf = dis(world_.get_random_engine());
  // auto t_serv = -std::log(1.0 - pf) / serve_intensity;
  auto t_serv = exponential(world_.get_random_engine());
  load.end_time = load.send_time + static_cast<Time>(t_serv);
  // load.end_time =
  // load.send_time + gsl::narrow<Time>(load.size / serve_intensity);
}

void Group::add_load(Load load)
{
  served_load_size += load.size;
  load.served_by.reset(this);
  set_end_time(load);
  world_.queue_load_to_serve(load);
}

bool Group::serve(Load load)
{
  if (can_serve(load)) {
    debug_print("{} Serving load: {}\n", *this, load);
    add_load(load);
    return true;
  }
  debug_print("{} Forwarding load: {}\n", *this, load);
  return forward(load);
}

bool Group::can_serve(const Load &load)
{
  return served_load_size + load.size <= capacity_;
}

bool Group::forward(Load load)
{
  // TODO(PW): make it more intelligent
  if (!next_groups.empty()) {
    return next_groups.front()->serve(load);
  }
  return loss_group.serve(load);
}

void Group::take_off(const Load &load)
{
  debug_print("{} Load has been served: {}\n", *this, load);
  served_load_size -= load.size;
  total_served_load_size += load.size;
  total_served_load_count++;
}

Stats Group::get_stats()
{
  return {loss_group.total_served_load_count, total_served_load_count,
          loss_group.total_served_load_size, total_served_load_size};
}

LossGroup::LossGroup(World &world) : id(world.get_unique_id()), world_(world)
{
}
bool LossGroup::serve(Load load)
{
  debug_print("{} Load droped. {}\n", *this, load);
  total_served_load_size += load.size;
  total_served_load_count++;
  return false;
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Group &group)

{
  f.writer().write("[Group {}, cap={}/{}]", group.id, group.served_load_size,
                   group.capacity_);
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LossGroup &loss_group)
{
  f.writer().write("[LossGroup {}]", loss_group.id);
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Stats &stats)
{
  auto loss_probability = static_cast<double>(stats.total_lost) /
                           (stats.total_served + stats.total_lost);
  auto loss_probability_size =
      static_cast<double>(stats.total_lost_size) /
      (stats.total_served_size + stats.total_lost_size);
  f.writer().write("served/lost: {}/{}, {}u/{}u. Pb: {}, {}",
                   stats.total_served, stats.total_lost,
                   stats.total_served_size, stats.total_lost_size,
                   loss_probability, loss_probability_size);
}
