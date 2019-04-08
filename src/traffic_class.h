#pragma once

#include "types/types.h"

#include <boost/container/flat_map.hpp>
#include <fmt/format.h>
#include <unordered_map>

struct TrafficClass
{
  TrafficClassId        id;
  Simulation::Intensity source_intensity;
  Simulation::Intensity serve_intensity;
  Size                  size;
  Length                max_path_length;
};

bool operator==(const TrafficClass &tc1, const TrafficClass &tc2);
using TrafficClasses = boost::container::flat_map<TrafficClassId, TrafficClass>;

// void
// format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const TrafficClass &tc);

namespace fmt {
template <>
struct formatter<TrafficClass>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TrafficClass &tc, FormatContext &ctx)
  {
    return format_to(
        ctx.begin(),
        "[TC: id={:>2}, l={:<8} u={}, size={}]",
        tc.id,
        tc.source_intensity,
        tc.serve_intensity,
        tc.size);
  }
};
} // namespace fmt
