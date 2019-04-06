#pragma once

#include "load.h"
#include "overflow_policy/overflow_policy.h"
#include "stats.h"
#include "traffic_class.h"
#include "types/types.h"
#include "world.h"

#include <algorithm>
#include <boost/container/flat_map.hpp>
#include <queue>
#include <random>
#include <range/v3/numeric.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Simulation {
struct CanServeRecursiveResult
{
  bool recursively;
  bool local;
       operator bool() { return recursively || local; }
};

struct CanServeResult
{
  bool              can_serve;
  CompressionRatio *compression_ratio;
  size_t            bucket;
};

using CompressionRatios =
    boost::container::flat_map<Capacity, CompressionRatio, std::greater<Capacity>>;

std::vector<Capacity>
operator-(const std::vector<Capacity> &capacities, const std::vector<Size> &sizes);

struct Group
{
  GroupId               id{};
  const GroupName       name_;
  std::vector<Capacity> capacity_;
  Capacity              total_capacity_ = ranges::accumulate(capacity_, Capacity{});
  std::vector<Size>     size_{};
  Layer                 layer_;

  GroupStatistics stats_{};

  World *                         world_ = nullptr;
  const TrafficClasses *          traffic_classes_{};
  std::vector<Group *>            next_groups_{};
  std::unique_ptr<OverflowPolicy> overflow_policy_;

  boost::container::flat_map<TrafficClassId, CompressionRatios> tcs_compression_{};
  std::unordered_set<TrafficClassId>                            tcs_block_{};

  std::exponential_distribution<time_type<>> exponential{};

  void                        set_world(World &world);
  void                        set_traffic_classes(const TrafficClasses &traffic_classes);
  void                        set_overflow_policy(std::unique_ptr<OverflowPolicy> overflow_policy);
  void                        add_next_group(Group &group);
  const std::vector<Group *> &next_groups() { return next_groups_; }

  void add_compression_ratio(
      TrafficClassId  tc_id,
      Capacity        threshold,
      Size            size,
      IntensityFactor intensity_factor);
  void block_traffic_class(TrafficClassId tc_id);

  void set_end_time(Load &load, IntensityFactor intensity_factor);

  bool forward(Load load);

  std::vector<Capacity> free_capacity() { return capacity_ - size_; }
  std::vector<Capacity> capacity() { return capacity_; }
  Layer                 layer() { return layer_; }

  CanServeResult          can_serve(const TrafficClass &tc);
  CanServeResult          can_serve(TrafficClassId tc_id);
  CanServeRecursiveResult can_serve_recursive(const TrafficClass &tc, Path &path);

  void block_recursive(TrafficClassId tc_id, const Load &load);
  void unblock_recursive(TrafficClassId tc_id, const Load &load);
  void block(TrafficClassId tc_id, const Load &load);
  void unblock(TrafficClassId tc_id, const Load &load);
  void update_block_stat(const Load &load);
  void update_unblock_stat(const Load &load);

  Group(GroupName name, Capacity capacity, Layer layer);
  Group(GroupName name, Capacity capacity);
  Group(const Group &) = delete;
  Group &operator=(const Group &) = delete;

  bool try_serve(Load load);
  void take_off(const Load &load);
  void drop(const Load &load);

  void notify_on_request_service_end(LoadServiceEndEvent *event);

  Stats            get_stats(Duration duration);
  const GroupName &name() const { return name_; }
};

} // namespace Simulation

namespace fmt {
template <>
struct formatter<Simulation::Group>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::Group &group, FormatContext &ctx)
  {
    return format_to(
        ctx.begin(),
        "t={} [Group {} V={}/{}, L{}]",
        group.world_->get_current_time(),
        group.name_,
        fmt::join(group.size_, ","),
        fmt::join(group.capacity_, ","),
        group.layer_);
  }
};

} // namespace fmt
