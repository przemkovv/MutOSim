#pragma once

#include "types.h"
#include "traffic_class.h"
#include <experimental/memory>
#include <fmt/format.h>
#include <gsl/gsl>

struct Group;
class SourceStream;

struct Load {
  LoadId id;
  TrafficClassId tc_id;
  Time send_time;
  Size size;
  Time end_time{-1};
  bool drop = false;

  std::experimental::observer_ptr<Group> served_by{};
  std::experimental::observer_ptr<SourceStream> produced_by{};
  std::experimental::observer_ptr<Group> target_group{};
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Load &load);
