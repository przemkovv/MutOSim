
#include "group.h"
#include "logger.h"
#include "source_stream/source_stream.h"

#include <cmath>
#include <gsl/gsl>

Group::Group(GroupName name, Capacity capacity)
  : name_(std::move(name)),
    capacity_(capacity),
    loss_group(name + "_LG", served_by_tc)
{
}

void Group::set_end_time(Load &load)
{
  auto &tcs = *traffic_classes_.get();
  auto serve_intensity = tcs[ts::get(load.tc_id)].serve_intensity;
  auto params = decltype(exponential)::param_type(ts::get(serve_intensity));
  exponential.param(params);

  Duration t_serv{exponential(world_->get_random_engine())};
  load.end_time = load.send_time + t_serv;
}

void Group::add_next_group(gsl::not_null<Group *> group)
{
  next_groups_.emplace_back(make_observer(group.get()));
}

void Group::set_traffic_classes(const TrafficClasses &traffic_classes)
{
  traffic_classes_ = make_observer(&traffic_classes);
}
// void Group::add_traffic_class(const TrafficClass &tc)
// {
// traffic_classes[tc.source_id] = tc;
// blocked_by_tc.emplace(tc.source_id, BlockStats{});
// served_by_tc.emplace(tc.source_id, LostServedStats{});
// }

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
  auto &served = served_by_tc[load.tc_id].served;
  served.size += load.size;
  served.count++;
}

void Group::update_block_stat(const Load &load)
{
  for (const auto &   tc  : *traffic_classes_) {
    if (can_serve(tc.size)) {
      unblock(tc.id, load);
    } else {
      block(tc.id, load);
    }
  }
}
void Group::block(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = blocked_by_tc[tc_id];
  if (!block_stats.is_blocked) {
    block_stats.is_blocked = true;
    block_stats.start_of_block = load.send_time;
    debug_print("{} Load: {}, Blocking bt={}, sobt={}\n", *this, load,
                block_stats.block_time, block_stats.start_of_block);
  }
}

void Group::unblock(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = blocked_by_tc[tc_id];
  if (block_stats.is_blocked) {
    block_stats.is_blocked = false;
    auto block_time = load.end_time - block_stats.start_of_block;
    block_stats.block_time += block_time;
    debug_print("{} Load: {}, Unblocking bt={}, dt={}\n", *this, load,
                block_stats.block_time, block_time);
  }
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
  Stats stats;

  if (loss_group.served_by_tc.size() != served_by_tc.size()) {
    print("{} Stats: source number in the group and loss group is different.\n",
          *this);
  }
  auto sim_duration = Duration{world_->get_time()};
  for (auto & [ tc_id, load_stats ] : served_by_tc) {
    auto &serve_stats = served_by_tc[tc_id];
    stats.by_traffic_class[tc_id] = {{serve_stats.lost, serve_stats.served},
                                  blocked_by_tc[tc_id].block_time,
                                  sim_duration};
    stats.total += serve_stats;
  }
  return stats;
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Group &group)

{
  f.writer().write("[Group {}, cap={}/{}]", group.name_, group.size_,
                   group.capacity_);
}
