#pragma once

#include "load.h"
#include "stats.h"
#include "traffic_class.h"
#include "types.h"
#include "world.h"
#include "loss_group.h"

#include <experimental/memory>
#include <gsl/gsl>
#include <queue>
#include <random>
#include <unordered_map>
#include <vector>

struct Group {
  GroupId id;
  const GroupName name_;
  Capacity capacity_;
  Size size_;

  std::unordered_map<TrafficClassId, LostServedStats> served_by_tc;
  std::unordered_map<TrafficClassId, BlockStats> blocked_by_tc;
  observer_ptr<const TrafficClasses> traffic_classes_;

  std::exponential_distribution<long double> exponential{};

  observer_ptr<World> world_;

  void set_world(gsl::not_null<World *> world)
  {
    world_ = make_observer(world.get());
  }

  std::vector<observer_ptr<Group>> next_groups_{};
  LossGroup loss_group;

  void add_next_group(gsl::not_null<Group *> group);

  void set_end_time(Load &load);
  bool forward(Load load);

  bool can_serve(const Size &load_size);
  void block(TrafficClassId tc_id, const Load &load);
  void unblock(TrafficClassId tc_id, const Load &load);
  void update_block_stat(const Load &load);

  Group(GroupName name, Capacity capacity);

  void set_traffic_classes(const TrafficClasses& traffic_classes);

  bool try_serve(Load load);
  void take_off(const Load &load);

  void notify_on_serve(LoadServeEvent *event);

  Stats get_stats();
  const GroupName &get_name() { return name_; }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Group &group);

