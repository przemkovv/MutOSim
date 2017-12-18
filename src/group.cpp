
#include "group.h"
#include "logger.h"
#include "math.h"
#include "source_stream.h"

#include <cmath>
#include <gsl/gsl>

Group::Group(const Name &name, Size capacity, Intensity serve_intensity)
  : name_(name),
    capacity_(capacity),
    serve_intensity_(serve_intensity),
    loss_group(name + "_LG")
{
}

void Group::set_end_time(Load &load)
{
  // auto pf = dis(world_.get_random_engine());
  // auto t_serv = -std::log(1.0 - pf) / serve_intensity;
  auto t_serv = exponential(world_->get_random_engine());
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
  world_->schedule(std::make_unique<LoadServeEvent>(
      world_->get_uuid(), load, group_serve_event_callback));
}

bool Group::serve(Load load)
{
  if (can_serve(load.size)) {
    debug_print("{} Serving load: {}\n", *this, load);
    add_load(load);
    if (is_blocked()) {
      block_stats_.start_of_block = load.send_time;
      debug_print("{} Blocking bt={}, sobt={}\n", *this,
                  block_stats_.block_time, block_stats_.start_of_block);
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
    auto block_time = load.end_time - block_stats_.start_of_block;
    block_stats_.block_time += block_time;
    debug_print("{} Unblocking bt={}, dt={}\n", *this, block_stats_.block_time,
                block_time);
  }
  size_ -= load.size;
  total_served.size += load.size;
  total_served.count++;
  served_by_source[load.produced_by->id].size += load.size;
  served_by_source[load.produced_by->id].count++;

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
  if (load.drop) {
    return loss_group.serve(load);
  }
  // TODO(PW): make it more intelligent
  if (!next_groups_.empty()) {
    return next_groups_.front()->serve(load);
  }
  return loss_group.serve(load);
}

Stats Group::get_stats()
{
  Stats stats{{loss_group.total_served, total_served},
              block_stats_.block_time,
              world_->get_time(),
              {}};

  if (loss_group.served_by_source.size() != served_by_source.size()) {
    print("{} Stats: source number in the group and loss group is different.\n",
          *this);
  }
  for (auto & [ source_id, load_stats ] : served_by_source) {
    stats.by_source[source_id] = {loss_group.served_by_source[source_id],
                                  served_by_source[source_id]};
  }
  return stats;
}

LossGroup::LossGroup(const Name &name) : name_(name)
{
}
bool LossGroup::serve(Load load)
{
  debug_print("{} Load droped. {}\n", *this, load);
  total_served.size += load.size;
  total_served.count++;
  served_by_source[load.produced_by->id].size += load.size;
  served_by_source[load.produced_by->id].count++;
  return false;
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Group &group)

{
  f.writer().write("[Group {}, cap={}/{}]", group.name_, group.size_,
                   group.capacity_);
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LossGroup &loss_group)
{
  f.writer().write("[LossGroup {}, lost={}]", loss_group.name_,
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
                const LostServedStats &stats)
{
  auto loss_ratio =
      Math::ratio_to_sum<double>(stats.lost.count, stats.served.count);
  auto loss_ratio_size =
      Math::ratio_to_sum<double>(stats.lost.size, stats.served.size);

  f.writer().write("served/lost: {} / {}. P_loss: {} ({})", stats.served,
                   stats.lost, loss_ratio, loss_ratio_size);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Stats &stats)
{
  f.writer().write("{}, P_block: {}", stats.total,
                   stats.block_time / stats.simulation_time);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const std::map<Uuid, LostServedStats> &lost_served_stats)
{
  for (auto & [ source_id, stats ] : lost_served_stats) {
    f.writer().write("source_id={}: {}", source_id, stats);
  }
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const std::map<Uuid, LoadStats> &served_by_source)
{
  for (auto & [ source_id, stats ] : served_by_source) {
    f.writer().write("source_id={}: {}", source_id, stats);
  }
}
