#pragma once

#include "load.h"
#include "types.h"
#include "world.h"

#include <experimental/memory>
#include <gsl/gsl>
#include <queue>
#include <random>
#include <vector>

using std::experimental::make_observer;
using std::experimental::observer_ptr;

struct LoadStats {
  Size count;
  Size size;
};

struct Stats {
  LoadStats total_lost;
  LoadStats total_served;
  Duration block_time;
  Duration simulation_time;
};

struct LossGroup {
  const Name name_;

  LoadStats total_served{0, 0};

  observer_ptr<World> world_;

  void set_world(gsl::not_null<World*> world) {
    world_ = make_observer(world.get());
  }

  LossGroup(const Name &name);
  bool serve(Load load);
};

struct Group {
  const Name name_;
  Size capacity_ = 1;
  Size size_ = 0;

  LoadStats total_served{0, 0};

  Duration block_time_ = 0;
  Time start_of_block_;

  Intensity serve_intensity_ = 1.0;

  std::uniform_real_distribution<> dis{0.0, 1.0};
  std::exponential_distribution<> exponential{serve_intensity_};

  observer_ptr<World> world_;

  void set_world(gsl::not_null<World*> world) {
    world_ = make_observer(world.get());
    loss_group.set_world(world);
  }

  std::vector<observer_ptr<Group>> next_groups_{};
  LossGroup loss_group;

  void add_next_group(gsl::not_null<Group *> group);
  void set_end_time(Load &load);
  void add_load(Load load);
  bool forward(Load load);
  bool can_serve(const Size &load_size);
  bool is_blocked();

  Group(const Name &name, Size capacity, Intensity serve_intensity);

  bool serve(Load load);
  void take_off(const Load &load);

  Stats get_stats();
  const Name& get_name() { return name_; }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Group &group);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LossGroup &loss_group);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Stats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LoadStats &load_stats);
