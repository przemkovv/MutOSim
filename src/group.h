#pragma once

#include "load.h"
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

using std::experimental::make_observer;
using std::experimental::observer_ptr;

struct LossGroup {
  const GroupName name_;

  LoadStats total_served{Count(0), Size(0)};
  std::unordered_map<SourceId, LoadStats> served_by_source;

  observer_ptr<World> world_;

  void set_world(gsl::not_null<World *> world)
  {
    world_ = make_observer(world.get());
  }

  LossGroup(GroupName name);
  bool serve(Load load);
};

struct Group {
  GroupId id;
  const GroupName name_;
  Capacity capacity_;
  Size size_;

  LoadStats total_served{Count{0}, Size{0}};
  std::unordered_map<SourceId, LoadStats> served_by_source;

  std::unordered_map<SourceId, BlockStats> blocked_by_source;

  std::uniform_real_distribution<> dis{0.0, 1.0};
  std::exponential_distribution<long double> exponential{};

  std::unordered_map<SourceId, TrafficClass> traffic_classes;

  observer_ptr<World> world_;

  void set_world(gsl::not_null<World *> world)
  {
    world_ = make_observer(world.get());
    loss_group.set_world(world);
  }

  std::vector<observer_ptr<Group>> next_groups_{};
  LossGroup loss_group;

  void add_next_group(gsl::not_null<Group *> group);
  void set_end_time(Load &load);
  bool forward(Load load);
  bool can_serve(const Load &load);
  bool can_serve(const Size &load_size);
  bool is_blocked(const Load &load);
  void block(SourceId source_id, const Load &load);
  void unblock(SourceId source_id, const Load &load);
  void update_block_stat(const Load& load);

  Group(GroupName name, Capacity capacity);

  void add_traffic_class(const TrafficClass &tc);

  bool try_serve(Load load);
  void take_off(const Load &load);

  void notify_on_serve(LoadServeEvent *event);

  Stats get_stats();
  const GroupName &get_name() { return name_; }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Group &group);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LossGroup &loss_group);
