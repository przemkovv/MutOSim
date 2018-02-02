#pragma once

#include "types.h"
#include <fmt/format.h>

#include <unordered_map>
#include <boost/container/flat_map.hpp>

struct TrafficClass {
  TrafficClassId id;
  Intensity source_intensity;
  Intensity serve_intensity;
  Size size;
  Length max_path_length;
};

using TrafficClasses = boost::container::flat_map<TrafficClassId, TrafficClass>;

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const TrafficClass &tc);
