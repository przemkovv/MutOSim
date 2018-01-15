
#include "group.h"
#include "logger.h"
#include "source_stream/source_stream.h"

#include <algorithm>
#include <cmath>
#include <gsl/gsl>

Group::Group(GroupName name, Capacity capacity)
  : name_(std::move(name)), capacity_(capacity)
{
}

void Group::set_end_time(Load &load)
{
  const auto &tcs = *traffic_classes_;
  const auto serve_intensity = tcs.at(load.tc_id).serve_intensity;
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

void Group::notify_on_service_end(LoadServiceEndEvent *event)
{
  take_off(event->load);
}

bool Group::try_serve(Load load)
{
  if (can_serve(load.size)) {
    debug_print("{} Start serving request: {}\n", *this, load);
    size_ += load.size;
    load.served_by.reset(this);
    set_end_time(load);

    update_block_stat(load);
    world_->update_block_stat(load);

    world_->schedule(std::make_unique<LoadServiceEndEvent>(world_->get_uuid(), load));
    return true;
  }
  debug_print("{} Forwarding request: {}\n", *this, load);
  load.path.emplace_back(make_observer(this));
  return forward(load);
}

void Group::take_off(const Load &load)
{
  debug_print("{} Request has been served: {}\n", *this, load);
  size_ -= load.size;
  update_unblock_stat(load);
  world_->update_block_stat(load);
  auto &served = served_by_tc[load.tc_id].served;
  served.size += load.size;
  served.count++;
}
void Group::drop(const Load &load)
{
  debug_print("{} Request has been dropped: {}\n", *this, load);
  auto &lost = served_by_tc[load.tc_id].lost;
  lost.size += load.size;
  lost.count++;
}

void Group::update_unblock_stat(const Load &load)
{
  for (const auto &[tc_id, tc] : *traffic_classes_) {
    Path path;
    if (can_serve_recursive(tc.size, path)) {
      unblock(tc.id, load);
    }
    assert(path.size() == 0);
  }
}
void Group::update_block_stat(const Load &load)
{
  for (const auto &[tc_id, tc] : *traffic_classes_) {
    Path path;
    if (!can_serve_recursive(tc.size, path)) {
      block(tc.id, load);
    }
    assert(path.size() == 0);
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

bool Group::can_serve_recursive(const Size &load_size, Path &path)
{
  if (can_serve(load_size)) {
    return true;
  }
  path.emplace_back(make_observer(this));
  auto pop_on_exit = gsl::finally([&path]() { path.pop_back(); });

  for (const auto &next_group : next_groups_) {
    if (find(begin(path), end(path), next_group) == end(path)) {
      return next_group->can_serve_recursive(load_size, path);
    }
  }
  return false;
}

bool Group::forward(Load load)
{
  if (load.drop) {
    drop(load);
    return false;
  }
  // TODO(PW): make it more intelligent
  for (const auto &next_group : next_groups_) {
    if (find(begin(load.path), end(load.path), next_group) == end(load.path)) {
      auto is_served = next_group->try_serve(load);
      if (!is_served) {
        drop(load);
      }
      return is_served;
    }
  }
  drop(load);
  return false;
}

Stats Group::get_stats()
{
  Stats stats;

  auto sim_duration = Duration{world_->get_time()};
  for (auto &[tc_id, load_stats] : served_by_tc) {
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
  f.writer().write("t={} [Group {}, cap={}/{}]", group.world_->get_current_time(),
                   group.name_, group.size_, group.capacity_);
}
