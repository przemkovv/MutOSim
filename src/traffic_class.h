#pragma once

#include "types.h"
#include <fmt/format.h>

#include <unordered_map>

struct TrafficClass {
  TrafficClassId id;
  Intensity source_intensity;
  Intensity serve_intensity;
  Size size;
  Length max_path_length;
};

using TrafficClasses = std::unordered_map<TrafficClassId, TrafficClass>;

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const TrafficClass &tc);
