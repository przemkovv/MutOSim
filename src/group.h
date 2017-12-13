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

struct Stats {
  Size total_lost;
  Size total_served;
  Size total_lost_size;
  Size total_served_size;
  Duration block_time;
};

struct LossGroup {
  Uuid id;

  Size total_served_load_size = 0;
  Size total_served_load_count = 0;

  World &world_;

  LossGroup(World &world);
  bool serve(Load load);
};

struct Group {
  Uuid id;
  Size capacity_ = 1;
  Size served_load_size = 0;

  Size total_served_load_size = 0;
  Size total_served_load_count = 0;

  Duration block_time_ = 0;
  Time start_of_block_;

  Intensity serve_intensity_ = 1.0;

  std::uniform_real_distribution<> dis{0.0, 1.0};
  std::exponential_distribution<> exponential{serve_intensity_};

  World &world_;

  std::vector<observer_ptr<Group>> next_groups_{};
  LossGroup loss_group;

  void add_next_group(gsl::not_null<Group *> group);
  void set_end_time(Load &load);
  void add_load(Load load);
  bool forward(Load load);
  bool can_serve(const Load &load);
  bool is_blocked();

  Group(World &world, Size capacity, Intensity serve_intensity);

  bool serve(Load load);
  void take_off(const Load &load);

  Stats get_stats();
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
