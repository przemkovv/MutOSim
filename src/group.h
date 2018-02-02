#pragma once

#include "load.h"
#include "overflow_policy/overflow_policy.h"
#include "stats.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"

#include <queue>
#include <random>
#include <unordered_map>
#include <boost/container/flat_map.hpp>
#include <vector>

using overflow_policy::OverflowPolicy;

struct Group {
  GroupId id;
  const GroupName name_;
  Capacity capacity_;
  Size size_;
  Layer layer_;


  boost::container::flat_map<TrafficClassId, LostServedStats> served_by_tc;
  boost::container::flat_map<TrafficClassId, BlockStats> blocked_by_tc;

  World *world_;
  const TrafficClasses *traffic_classes_;
  std::vector<Group *> next_groups_{};
  std::unique_ptr<overflow_policy::OverflowPolicy> overflow_policy_;

  std::exponential_distribution<time_type> exponential{};


  void set_world(World& world);
  void set_traffic_classes(const TrafficClasses &traffic_classes);
  void set_overflow_policy(std::unique_ptr<OverflowPolicy> overflow_policy);
  void add_next_group(Group& group);

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
