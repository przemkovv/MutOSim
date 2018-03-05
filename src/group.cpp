
#include "group.h"
#include "logger.h"
#include "overflow_policy/factory.h"
#include "source_stream/source_stream.h"

#include <algorithm>
#include <cmath>
#include <fmt/ostream.h>
#include <gsl/gsl>

Group::Group(GroupName name, Capacity capacity) : Group(name, capacity, 0)
{
}
Group::Group(GroupName name, Capacity capacity, Layer layer)
  : name_(std::move(name)),
    capacity_(capacity),
    layer_(layer),
    overflow_policy_(overflow_policy::make_overflow_policy("default", *this))
{
  assert(layer_ < MaxLayersNumber);
}

void Group::set_world(World &world)
{
  world_ = &world;
  overflow_policy_->set_world(world);
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

void Group::add_next_group(Group &group)
{
  next_groups_.emplace_back(&group);
}

void Group::set_traffic_classes(const TrafficClasses &traffic_classes)
{
  traffic_classes_ = &traffic_classes;
}

void Group::set_overflow_policy(
    std::unique_ptr<overflow_policy::OverflowPolicy> overflow_policy)
{
  overflow_policy_ = std::move(overflow_policy);
}

void Group::add_compression_ratio(TrafficClassId tc_id,
                                  Capacity threshold,
                                  Size size,
                                  IntensityFactor intensity_factor)
{
  auto &crs = tcs_compression_[tc_id];
  crs.emplace(threshold, CompressionRatio{size, intensity_factor});
}

void Group::notify_on_service_end(LoadServiceEndEvent *event)
{
  take_off(event->load);
}

bool Group::try_serve(Load load)
{
  load.served_by.emplace_back(this);

  if (can_serve(load.size)) {
    debug_print("{} Start serving request: {}\n", *this, load);
    size_ += load.size;
    set_end_time(load);

    update_block_stat(load);
    world_->update_block_stat(load);

    world_->schedule(std::make_unique<LoadServiceEndEvent>(world_->get_uuid(), load));
    return true;
  }
  debug_print("{} Forwarding request: {}\n", *this, load);
  return forward(load);
}

void Group::take_off(const Load &load)
{
  debug_print("{} Request has been served: {}\n", *this, load);
  size_ -= load.size;
  update_unblock_stat(load);
  world_->update_unblock_stat(load);
  stats_.served_by_tc[load.tc_id].serve(load);
}
void Group::drop(const Load &load)
{
  debug_print("{} Request has been dropped: {}\n", *this, load);
  stats_.served_by_tc[load.tc_id].drop(load);
}

void Group::update_unblock_stat(const Load &load)
{
  for (const auto &[tc_id, tc] : *traffic_classes_) {
    std::ignore = tc_id;
    Path path; // = load.path; // NOTE(PW): should be considered length of the current
               // load path?
    if (can_serve_recursive(tc, path)) {
      unblock(tc.id, load);
    }
    assert(path.size() == 0 /*load.path.size() */);
  }
}
void Group::update_block_stat(const Load &load)
{
  for (const auto &[tc_id, tc] : *traffic_classes_) {
    std::ignore = tc_id;
    Path path; // = load.path; // NOTE(PW): should be considered length of the current
               // load path?
    if (!can_serve_recursive(tc, path)) {
      block(tc.id, load);
    }
    assert(path.size() == 0 /*load.path.size() */);
  }
}
void Group::block(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = stats_.blocked_by_tc[tc_id];
  if (block_stats.try_block(load.send_time)) {
    debug_print("{} Load: {}, Blocking bt={}, sobt={}\n", *this, load,
                block_stats.block_time, block_stats.start_of_block);
  }
}

void Group::unblock(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = stats_.blocked_by_tc[tc_id];
  if (block_stats.try_unblock(load.end_time)) {
    debug_print("{} Load: {}, Unblocking bt={}\n", *this, load, block_stats.block_time);
  }
}

bool Group::can_serve(const Size &load_size)
{
  return size_ + load_size <= capacity_;
}

bool Group::can_serve_recursive(const TrafficClass &tc, Path &path)
{
  if (can_serve(tc.size)) {
    return true;
  }
  path.emplace_back(this);
  auto pop_on_exit = gsl::finally([&path]() { path.pop_back(); });

  if (path.size() >= tc.max_path_length) {
    return false;
  }

  for (const auto &next_group : next_groups_) {
    if (std::find(std::begin(path), std::end(path), next_group) == std::end(path)) {
      return next_group->can_serve_recursive(tc, path);
    }
  }
  return false;
}

bool Group::forward(Load load)
{
  // TODO(PW): if the max path has been reached, pass the load to the next layer
  if (load.drop ||
      load.served_by.size() >= traffic_classes_->at(load.tc_id).max_path_length) {
    drop(load);
    return false;
  }
  if (auto next_group = overflow_policy_->find_next_group(load); next_group) {
    auto is_served = (*next_group)->try_serve(load);
    if (!is_served) { // migrated load is considered as dropped by the local group
      drop(load);
    } else {
      stats_.served_by_tc[load.tc_id].forward(load);
    }
    return is_served;
  }
  drop(load);
  return false;
}

Stats Group::get_stats(Duration duration)
{
  return stats_.get_stats(duration);
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Group &group)

{
  f.writer().write("t={} [Group {} V={}/{}, L{}]", group.world_->get_current_time(),
                   group.name_, group.size_, group.capacity_, group.layer_);
}
