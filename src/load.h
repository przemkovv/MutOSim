#pragma once

#include "types.h"
#include <experimental/memory>

struct Group;

struct Load {
  Uuid id;
  Time send_time;
  Size size;
  Time end_time = -1;

  std::experimental::observer_ptr<Group> served_by{};
};

bool operator<(const Load &l1, const Load &l2);
