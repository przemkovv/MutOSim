#pragma once

#include "traffic_class.h"
#include "types.h"

#include <fmt/format.h>
#include <gsl/gsl>
#include <vector>

struct Group;
class SourceStream;

using Path = std::vector<Group *>;

struct Load {
  LoadId id;
  TrafficClassId tc_id;
  Time send_time;
  Size size;
  Time end_time{-1};
  bool drop = false;

  Path path{};

  Group *served_by{};
  SourceStream *produced_by{};
  Group *target_group{};
};

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const Load &load);
