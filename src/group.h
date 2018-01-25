#pragma once

#include "load.h"
#include "overflow_policy/overflow_policy.h"
#include "stats.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"

#include <experimental/memory>
#include <gsl/gsl>
#include <queue>
#include <random>
#include <unordered_map>
#include <vector>

using overflow_policy::OverflowPolicy;

struct Group {
  GroupId id;
  const GroupName name_;
  Capacity capacity_;
  Size size_;
  Layer layer_;

  std::unique_ptr<overflow_policy::OverflowPolicy> overflow_policy_;

  std::unordered_map<TrafficClassId, LostServedStats> served_by_tc;
  std::unordered_map<TrafficClassId, BlockStats> blocked_by_tc;
  observer_ptr<const TrafficClasses> traffic_classes_;

  std::exponential_distribution<time_type> exponential{};

  observer_ptr<World> world_;

  void set_world(gsl::not_null<World *> world);

  std::vector<observer_ptr<Group>> next_groups_{};

  void add_next_group(gsl::not_null<Group *> group);

  void set_end_time(Load &load);
  bool forward(Load load);

  Size free_capacity() { return capacity_ - size_; }
  bool can_serve(const Size &load_size);
  bool can_serve_recursive(const TrafficClass &tc, Path &path);
  void block(TrafficClassId tc_id, const Load &load);
  void unblock(TrafficClassId tc_id, const Load &load);
  void update_block_stat(const Load &load);
  void update_unblock_stat(const Load &load);

  Group(GroupName name, Capacity capacity, Layer layer);
  Group(GroupName name, Capacity capacity);

  void reset();
  void set_traffic_classes(const TrafficClasses &traffic_classes);

  void set_overflow_policy(std::unique_ptr<OverflowPolicy> overflow_policy);

  bool try_serve(Load load);
  void take_off(const Load &load);
  void drop(const Load &load);

  void notify_on_service_end(LoadServiceEndEvent *event);

  Stats get_stats();
  const GroupName &get_name() const { return name_; }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Group &group);
