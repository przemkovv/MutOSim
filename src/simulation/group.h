#pragma once

#include "load.h"
#include "overflow_policy/overflow_policy.h"
#include "stats.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"

#include <boost/container/flat_map.hpp>
#include <queue>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using overflow_policy::OverflowPolicy;


struct CanServeResult {
  bool recursively;
  bool local;
  operator bool() { return recursively || local; }
};

using CompressionRatios =
    boost::container::flat_map<Capacity, CompressionRatio, std::greater<Capacity>>;

struct Group {
  GroupId id{};
  const GroupName name_;
  Capacity capacity_;
  Size size_{};
  Layer layer_;

  GroupStatistics stats_{};

  World *world_ = nullptr;
  const TrafficClasses *traffic_classes_{};
  std::vector<Group *> next_groups_{};
  std::unique_ptr<overflow_policy::OverflowPolicy> overflow_policy_;

  boost::container::flat_map<TrafficClassId, CompressionRatios> tcs_compression_{};
  std::unordered_set<TrafficClassId> tcs_block_{};

  std::exponential_distribution<time_type> exponential{};

  void set_world(World &world);
  void set_traffic_classes(const TrafficClasses &traffic_classes);
  void set_overflow_policy(std::unique_ptr<OverflowPolicy> overflow_policy);
  void add_next_group(Group &group);
  const std::vector<Group *> &next_groups() { return next_groups_; }

  void add_compression_ratio(
      TrafficClassId tc_id,
      Capacity threshold,
      Size size,
      IntensityFactor intensity_factor);
  void block_traffic_class(TrafficClassId tc_id);

  void set_end_time(Load &load, IntensityFactor intensity_factor);

  bool forward(Load load);

  Capacity free_capacity() { return capacity_ - size_; }
  Capacity capacity() { return capacity_; }
  Layer layer() { return layer_; }
  std::pair<bool, CompressionRatio *> can_serve(const TrafficClass &tc);
  std::pair<bool, CompressionRatio *> can_serve(TrafficClassId tc_id);
  CanServeResult can_serve_recursive(const TrafficClass &tc, Path &path);
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

  void notify_on_service_end(LoadServiceEndEvent *event);

  Stats get_stats(Duration duration);
  [[deprecated]] const GroupName &
  get_name() const { return name_; } const GroupName &name() const
  {
    return name_;
  }
};

void
format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const Group &group);
