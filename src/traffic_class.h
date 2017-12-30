#pragma once

#include "types.h"

struct TrafficClass {
  TrafficClassId id;
  Intensity source_intensity;
  Intensity serve_intensity;
  Size size;
};

using TrafficClasses = std::vector<TrafficClass>;

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const TrafficClass &tc);
