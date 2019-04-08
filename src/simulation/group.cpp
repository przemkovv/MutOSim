
#include "group.h"

#include "load_format.h"
#include "logger.h"
#include "overflow_policy/factory.h"
#include "source_stream/source_stream.h"
#include "types/types_format.h"

#include <algorithm>
#include <cmath>
#include <gsl/gsl>

namespace Simulation {
std::vector<Capacity>
operator-(
    const std::vector<Capacity> &capacities,
    const std::vector<Size> &    sizes)
{
  std::vector<Capacity> free_capacities(capacities.size());
  std::transform(
      begin(capacities),
      end(capacities),
      begin(sizes),
      begin(free_capacities),
      [](auto x, auto y) { return x - y; });
  return free_capacities;
}

Group::Group(GroupName name, Capacity capacity) : Group(name, capacity, 0)
{
}
Group::Group(GroupName name, Capacity capacity, Layer layer)
  : name_(std::move(name)),
    capacity_({capacity}),
    size_(capacity_.size()),
    layer_(layer),
    overflow_policy_(make_overflow_policy("default", *this))
{
  ASSERT(
      layer_ < MaxLayersNumber,
      "The layer number {} of group {} should be lower than {}.",
      layer,
      name,
      MaxLayersNumber);
}

void
Group::set_world(World &world)
{
  world_ = &world;
  overflow_policy_->set_world(world);
}

void
Group::set_end_time(Load &load, IntensityFactor intensity_factor)
{
  const auto &tcs = *traffic_classes_;
  const auto  serve_intensity =
      intensity_factor * tcs.at(load.tc_id).serve_intensity;
  auto params = decltype(exponential)::param_type(ts::get(serve_intensity));
  exponential.param(params);

  Duration t_serv{exponential(world_->get_random_engine())};
  load.end_time = load.send_time + t_serv;
}

void
Group::add_next_group(Group &group)
{
  next_groups_.emplace_back(&group);
}

void
Group::set_traffic_classes(const TrafficClasses &traffic_classes)
{
  traffic_classes_ = &traffic_classes;
}

void
Group::set_overflow_policy(std::unique_ptr<OverflowPolicy> overflow_policy)
{
  overflow_policy_ = std::move(overflow_policy);
}

void
Group::add_compression_ratio(
    TrafficClassId  tc_id,
    Capacity        threshold,
    Size            size,
    IntensityFactor intensity_factor)
{
  auto &crs = tcs_compression_[tc_id];
  crs.emplace(threshold, CompressionRatio{size, intensity_factor});
}

void
Group::block_traffic_class(TrafficClassId tc_id)
{
  tcs_block_.insert(tc_id);
}

void
Group::notify_on_request_service_end(LoadServiceEndEvent *event)
{
  take_off(event->load);
}

bool
Group::try_serve(Load load)
{
  load.served_by.emplace_back(this);

  if (auto [ok, compression, bucket] = can_serve(load.tc_id); ok)
  {
    std::ignore = bucket;
    IntensityFactor intensity_factor{1.0l};
    if (compression)
    {
      load.size = compression->size;
      intensity_factor = compression->intensity_factor;
      load.compression_ratio = compression;
    }
    debug_print("{} Start serving request: {}\n", *this, load);
    size_[bucket] += load.size;
    load.bucket = bucket;
    set_end_time(load, intensity_factor);

    update_block_stat(load);

    world_->schedule(
        std::make_unique<LoadServiceEndEvent>(world_->get_uuid(), load));
    return true;
  }
  debug_print("{} Forwarding request: {}\n", *this, load);
  return forward(load);
}

void
Group::take_off(const Load &load)
{
  debug_print("{} Request has been served: {}\n", *this, load);
  size_[load.bucket] -= load.size;
  update_unblock_stat(load);
  stats_.served_by_tc[load.tc_id].serve(load);
}
void
Group::drop(const Load &load)
{
  debug_print("{} Request has been dropped: {}\n", *this, load);
  stats_.served_by_tc[load.tc_id].drop(load);
}

void
Group::update_unblock_stat(const Load &load)
{
  for (const auto &[tc_id, tc] : *traffic_classes_)
  {
    std::ignore = tc_id;
    Path path; // = load.path; // NOTE(PW): should be considered length of the
               // current
    if (auto [recursive, local] = can_serve_recursive(tc, path); local)
    {
      unblock_recursive(tc.id, load);
      unblock(tc.id, load);
    }
    else if (recursive)
    {
      unblock_recursive(tc.id, load);
    }
    ASSERT(path.size() == 0 /*load.path.size() */, "Path should be empty.");
  }
}
void
Group::update_block_stat(const Load &load)
{
  for (const auto &[tc_id, tc] : *traffic_classes_)
  {
    std::ignore = tc_id;
    Path path; // = load.path; // NOTE(PW): should be considered length of the
               // current
    if (auto [recursive, local] = can_serve_recursive(tc, path);
        !local && recursive)
    {
      block(tc.id, load);
    }
    else if (!recursive)
    {
      block_recursive(tc.id, load);
      block(tc.id, load);
    }
    ASSERT(path.size() == 0 /*load.path.size() */, "Path should be empty.");
  }
}
void
Group::block(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = stats_.blocked_by_tc[tc_id];
  if (block_stats.try_block(load.send_time))
  {
    debug_print(
        "{} Load: {}, Blocking bt={}, sobt={}\n",
        *this,
        load,
        block_stats.block_time,
        block_stats.start_of_block);
  }
}

void
Group::unblock(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = stats_.blocked_by_tc[tc_id];
  if (block_stats.try_unblock(load.end_time))
  {
    debug_print(
        "{} Load: {}, Unblocking bt={}\n",
        *this,
        load,
        block_stats.block_time);
  }
}

void
Group::block_recursive(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = stats_.blocked_recursive_by_tc[tc_id];
  if (block_stats.try_block(load.send_time))
  {
    debug_print(
        "{} Load: {}, Blocking recursive bt={}, sobt={}\n",
        *this,
        load,
        block_stats.block_time,
        block_stats.start_of_block);
  }
}

void
Group::unblock_recursive(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = stats_.blocked_recursive_by_tc[tc_id];
  if (block_stats.try_unblock(load.end_time))
  {
    debug_print(
        "{} Load: {}, Unblocking recursive bt={}\n",
        *this,
        load,
        block_stats.block_time);
  }
}
CanServeResult
Group::can_serve(TrafficClassId tc_id)
{
  return can_serve(traffic_classes_->at(tc_id));
}

CanServeResult
Group::can_serve(const TrafficClass &tc)
{
  if (tcs_block_.find(tc.id) != end(tcs_block_))
  {
    return {false, nullptr, 0};
  }
  if (const auto tc_compression_it = tcs_compression_.find(tc.id);
      tc_compression_it != end(tcs_compression_))
  {
    for (size_t bucket = {0}; bucket < size_.size(); ++bucket)
    {
      if (const auto cr_it = tc_compression_it->second.lower_bound(
              Capacity{get(size_[bucket])});
          cr_it != end(tc_compression_it->second))
      {
        // TODO(PW): verify if the condition is correct with multiple buckets
        return {size_[bucket] + cr_it->second.size <= capacity_[bucket],
                &cr_it->second,
                bucket};
      }
    }
  }
  for (size_t bucket = {0}; bucket < size_.size(); ++bucket)
  {
    if (size_[bucket] + tc.size <= capacity_[bucket])
    {
      return {true, nullptr, bucket};
    }
  }
  return {false, nullptr, 0};
}

CanServeRecursiveResult
Group::can_serve_recursive(const TrafficClass &tc, Path &path)
{
  if (auto [ok, compression, bucket] = can_serve(tc); ok)
  {
    std::ignore = compression;
    std::ignore = bucket;
    return {true, true};
  }
  path.emplace_back(this);
  auto pop_on_exit = gsl::finally([&path]() { path.pop_back(); });

  if (path.size() >= tc.max_path_length)
  {
    return {false, false};
  }

  for (const auto &next_group : next_groups_)
  {
    if (std::find(std::begin(path), std::end(path), next_group)
        == std::end(path))
    {
      return {bool(next_group->can_serve_recursive(tc, path)), false};
    }
  }
  return {false, false};
}

bool
Group::forward(Load load)
{
  // TODO(PW): if the max path has been reached, pass the load to the next
  // layer
  if (load.drop
      || load.served_by.size()
             >= traffic_classes_->at(load.tc_id).max_path_length)
  {
    drop(load);
    return false;
  }
  if (auto next_group = overflow_policy_->find_next_group(load); next_group)
  {
    auto is_served = (*next_group)->try_serve(load);
    if (!is_served)
    { // migrated load is considered as dropped by the local group
      drop(load);
    }
    else
    {
      stats_.served_by_tc[load.tc_id].forward(load);
    }
    return is_served;
  }
  drop(load);
  return false;
}

Stats
Group::get_stats(Duration duration)
{
  return stats_.get_stats(duration);
}

//----------------------------------------------------------------------

} // namespace Simulation
