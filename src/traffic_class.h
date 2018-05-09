#pragma once

#include "types.h"

#include <boost/container/flat_map.hpp>
#include <fmt/format.h>
#include <unordered_map>

struct TrafficClass {
  TrafficClassId id;
  Intensity source_intensity;
  Intensity serve_intensity;
  Size size;
  Length max_path_length;
};

bool operator==(const TrafficClass &tc1, const TrafficClass &tc2);
using TrafficClasses = boost::container::flat_map<TrafficClassId, TrafficClass>;

void
format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const TrafficClass &tc);
