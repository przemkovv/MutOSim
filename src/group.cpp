
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
}

void Group::notify_on_serve(LoadServeEvent *event)
{
  take_off(event->load);
}

bool Group::try_serve(Load load)
{
  if (can_serve(load.size)) {
    debug_print("{} Serving load: {}\n", *this, load);
    size_ += load.size;
    load.served_by.reset(this);
    set_end_time(load);

    if (is_blocked()) {
      block_stats_.start_of_block = load.send_time;
      debug_print("{} Blocking bt={}, sobt={}\n", *this,
                  block_stats_.block_time, block_stats_.start_of_block);
    }

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
