
#include "group.h"
#include "logger.h"
#include "math.h"
#include "source_stream.h"

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

void Group::add_next_group(gsl::not_null<Group *> group)
{
  next_groups_.emplace_back(make_observer(group.get()));
}

static void group_serve_event_callback(World *, Event *e)
{
  auto load = static_cast<LoadServeEvent *>(e)->load;
  load.served_by->take_off(load);
}

void Group::add_load(Load load)
{
  size_ += load.size;
  load.served_by.reset(this);
  set_end_time(load);
  // auto on_process = [this](World *, Event *e) {
  // this->take_off(static_cast<LoadServeEvent *>(e)->load);
  // };
  world_.schedule(std::make_unique<LoadServeEvent>(world_.get_uuid(), load,
                                                   group_serve_event_callback));
  // world_.queue_load_to_serve(load);
}

bool Group::serve(Load load)
{
  if (can_serve(load.size)) {
    debug_print("{} Serving load: {}\n", *this, load);
    add_load(load);
    if (is_blocked()) {
      start_of_block_ = load.send_time;
      debug_print("{} Blocking bt={}, sobt={}\n", *this, block_time_, start_of_block_);
    }
    return true;
  }
  debug_print("{} Forwarding load: {}\n", *this, load);
  return forward(load);
}

void Group::take_off(const Load &load)
{
  debug_print("{} Load has been served: {}\n", *this, load);
  if (is_blocked()) {
    auto block_time = load.end_time - start_of_block_;
    block_time_ += block_time;
    debug_print("{} Unblocking bt={}, dt={}\n", *this, block_time_, block_time);
  }
  size_ -= load.size;
  total_served.size += load.size;
  total_served.count++;

  load.produced_by->notify_on_serve(load);
}

bool Group::is_blocked()
{
  return size_ == capacity_;
}

bool Group::can_serve(const Size &load_size)
{
  return size_ + load_size <= capacity_;
}

bool Group::forward(Load load)
{
  // TODO(PW): make it more intelligent
  if (!next_groups_.empty()) {
    return next_groups_.front()->serve(load);
  }
  return loss_group.serve(load);
}

Stats Group::get_stats()
{
  return {loss_group.total_served, total_served, block_time_,
          world_.get_time()};
}

LossGroup::LossGroup(World &world) : id(world.get_unique_id()), world_(world)
{
}
bool LossGroup::serve(Load load)
{
  debug_print("{} Load droped. {}\n", *this, load);
  total_served.size += load.size;
  total_served.count++;
  return false;
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Group &group)

{
  f.writer().write("[Group {}, cap={}/{}]", group.id, group.size_,
                   group.capacity_);
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LossGroup &loss_group)
{
  f.writer().write("[LossGroup {}, lost={}]", loss_group.id,
                   loss_group.total_served.count);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LoadStats &load_stats)
{
  f.writer().write("{} ({}u)", load_stats.count, load_stats.size);
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Stats &stats)
{
  auto loss_ratio = Math::ratio_to_sum<double>(stats.total_lost.count,
                                               stats.total_served.count);
  auto loss_ratio_size = Math::ratio_to_sum<double>(stats.total_lost.size,
                                                    stats.total_served.size);

  f.writer().write("served/lost: {} / {}. P_loss: {} ({}), P_block: {}",
                   stats.total_served, stats.total_lost, loss_ratio,
                   loss_ratio_size, stats.block_time / stats.simulation_time);
}
