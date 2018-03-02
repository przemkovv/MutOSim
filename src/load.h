#pragma once

#include "traffic_class.h"
#include "types.h"

#include <boost/container/small_vector.hpp>
#include <fmt/format.h>
#include <vector>

struct Group;
class SourceStream;

using Path = boost::container::small_vector<Group *, 5>;

struct Load {
  LoadId id{};
  TrafficClassId tc_id{};
  Time send_time{};
  Size size{};
  Time end_time{-1};
  bool drop = false;

  Path served_by{};
  SourceStream *produced_by = nullptr;
  Group *target_group = nullptr;
};

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const Load &load);
