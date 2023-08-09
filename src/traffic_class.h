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
  Simulation::Size      size;
  Length                max_path_length;
};

bool operator==(const TrafficClass &tc1, const TrafficClass &tc2);

using TrafficClasses = boost::container::flat_map<TrafficClassId, TrafficClass>;

template <>
struct fmt::formatter<TrafficClass>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TrafficClass &tc, FormatContext &ctx)
  {
    return fmt::format_to(
        ctx.out(),
        "[TC: id={:>2}, l={:<8} u={}, size={}]",
        tc.id,
        tc.source_intensity,
        tc.serve_intensity,
        tc.size);
  }
};
