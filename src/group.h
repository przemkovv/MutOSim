#pragma once

#include "load.h"
#include "types.h"
#include "stats.h"
#include "world.h"


#include <experimental/memory>
#include <gsl/gsl>
#include <queue>
#include <random>
#include <vector>
#include <unordered_map>

using std::experimental::make_observer;
using std::experimental::observer_ptr;


struct LossGroup {
  const Name name_;

  LoadStats total_served{0, 0};
  std::unordered_map<Uuid, LoadStats> served_by_source;

  observer_ptr<World> world_;

  void set_world(gsl::not_null<World *> world)
  {
    world_ = make_observer(world.get());
  }

  LossGroup(Name name);
  bool serve(Load load);
};

struct Group {
  const Name name_;
  Size capacity_ = 1;
  Size size_ = 0;

  LoadStats total_served{0, 0};
  std::unordered_map<Uuid, LoadStats> served_by_source;

  std::unordered_map<Uuid, BlockStats> blocked_by_source;
  BlockStats block_stats_;

  Intensity serve_intensity_ = 1.0;

  std::uniform_real_distribution<> dis{0.0, 1.0};
  std::exponential_distribution<> exponential{serve_intensity_};

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
  bool can_serve(const Size &load_size);
  bool is_blocked();

  Group(Name name, Size capacity, Intensity serve_intensity);

  bool try_serve(Load load);
  void take_off(const Load &load);

  void notify_on_serve(LoadServeEvent *event);

  Stats get_stats();
  const Name &get_name() { return name_; }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Group &group);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LossGroup &loss_group);

