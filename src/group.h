#pragma once

#include "load.h"
#include "world.h"
#include "types.h"

#include <experimental/memory>
#include <queue>
#include <vector>

using std::experimental::make_observer;
using std::experimental::observer_ptr;

struct BlockGroup {
  Uuid id;
  Size capacity = 1000;
  Size served_load_size = 0;

  World &world_;

  BlockGroup(World &world);
  void set_end_time(Load &load);
  void add_load(Load load);
  void serve(Load load);
  bool can_serve(const Load &load);
  void forward(Load load);
};

struct Group {
  Uuid id;
  Size capacity = 3;
  Size served_load_size = 0;

  float serve_intensity = 0.5f;

  World &world_;

  std::vector<observer_ptr<Group>> next_groups{};
  BlockGroup block_group;

  void set_end_time(Load &load);
  void add_load(Load load);
  void forward(Load load);
  bool can_serve(const Load &load);

  Group(World &world);

  void serve(Load load);
  void take_off(const Load& load);
};

