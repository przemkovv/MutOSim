
#include "group.h"
#include "logger.h"
#include "source_stream/source_stream.h"

#include <cmath>
#include <gsl/gsl>

Group::Group(GroupName name, Capacity capacity)
  : name_(std::move(name)), capacity_(capacity), loss_group(name + "_LG")
{
}

void Group::set_end_time(Load &load)
{
  auto serve_intensity = traffic_classes[load.produced_by->id].serve_intensity;
  auto params = decltype(exponential)::param_type(ts::get(serve_intensity));
  exponential.param(params);

  Duration t_serv{exponential(world_->get_random_engine())};
  load.end_time = load.send_time + t_serv;
}

void Group::add_next_group(gsl::not_null<Group *> group)
{
  next_groups_.emplace_back(make_observer(group.get()));
}

void Group::add_traffic_class(const TrafficClass &tc)
{
  traffic_classes[tc.source_id] = tc;
  blocked_by_source.emplace(tc.source_id, BlockStats{});
}

void Group::notify_on_serve(LoadServeEvent *event)
{
  take_off(event->load);
}

bool Group::try_serve(Load load)
{
  if (can_serve(load)) {
    debug_print("{} Serving load: {}\n", *this, load);
    size_ += load.size;
    load.served_by.reset(this);
    set_end_time(load);

    update_block_stat(load);

    world_->schedule(
        std::make_unique<LoadServeEvent>(world_->get_uuid(), load));
    return true;
  }
  debug_print("{} Forwarding load: {}\n", *this, load);
  return forward(load);
}

void Group::take_off(const Load &load)
{
  debug_print("{} Load has been served: {}\n", *this, load);
  size_ -= load.size;
  update_block_stat(load);
  total_served.size += load.size;
  total_served.count++;
  served_by_source[load.produced_by->id].size += load.size;
  served_by_source[load.produced_by->id].count++;
}

void Group::update_block_stat(const Load &load)
{
  for (const auto & [ source_id, traffic_class ] : traffic_classes) {
    if (can_serve(traffic_class.size)) {
      unblock(source_id, load);
    } else {
      block(source_id, load);
    }
  }
}
void Group::block(SourceId source_id, const Load &load)
{
  auto &block_stats = blocked_by_source[source_id];
  if (!block_stats.is_blocked) {
    block_stats.is_blocked = true;
    block_stats.start_of_block = load.send_time;
    debug_print("{} Blocking bt={}, sobt={}\n", *this, block_stats.block_time,
                block_stats.start_of_block);
  }
}

void Group::unblock(SourceId source_id, const Load &load)
{
  auto &block_stats = blocked_by_source[source_id];
  if (block_stats.is_blocked) {
    block_stats.is_blocked = false;
    block_stats.start_of_block = load.send_time;
    auto block_time = load.end_time - block_stats.start_of_block;
    block_stats.block_time += block_time;
    debug_print("{} Unblocking bt={}, dt={}\n", *this, block_stats.block_time,
                block_time);
  }
}

bool Group::is_blocked(const Load &load)
{
  return size_ + load.size > capacity_;
}

bool Group::can_serve(const Size &load_size)
{
  return size_ + load_size <= capacity_;
}
bool Group::can_serve(const Load &load)
{
  return size_ + load.size <= capacity_;
}

bool Group::forward(Load load)
{
  if (load.drop) {
    return loss_group.serve(load);
  }
  // TODO(PW): make it more intelligent
  if (!next_groups_.empty()) {
    return next_groups_.front()->try_serve(load);
  }
  return loss_group.serve(load);
}

Stats Group::get_stats()
{
  Stats stats{{loss_group.total_served, total_served},
              block_stats_.block_time,
              Duration{world_->get_time()},
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

//----------------------------------------------------------------------

LossGroup::LossGroup(GroupName name) : name_(std::move(name))
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

//----------------------------------------------------------------------

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
