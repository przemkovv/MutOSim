#pragma once

#include "load.h"
#include "types.h"
#include "world.h"

#include <experimental/memory>
#include <queue>
#include <random>
#include <vector>

using std::experimental::make_observer;
using std::experimental::observer_ptr;

struct Stats {
  Size total_blocked;
  Size total_served;
  Size total_blocked_size;
  Size total_served_size;
};

struct BlockGroup {
  Uuid id;

  Size total_served_load_size = 0;
  Size total_served_load_count = 0;

  World &world_;

  BlockGroup(World &world);
  bool serve(Load load);
};

struct Group {
  Uuid id;
  Size capacity = 1;
  Size served_load_size = 0;

  Size total_served_load_size = 0;
  Size total_served_load_count = 0;

  double serve_intensity = 1.0;

  std::uniform_real_distribution<> dis{0.0, 1.0};

  World &world_;

  std::vector<observer_ptr<Group>> next_groups{};
  BlockGroup block_group;

  void set_end_time(Load &load);
  void add_load(Load load);
  bool forward(Load load);
  bool can_serve(const Load &load);

  Group(World &world);

  bool serve(Load load);
  void take_off(const Load &load);

  Stats get_stats();
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Group &group);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const BlockGroup &block_group);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Stats &stats);
