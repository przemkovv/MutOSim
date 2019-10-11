
#pragma once

#include "load.h"

#include <fmt/format.h>

namespace fmt {
template <>
struct formatter<Simulation::Load>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::Load &load, FormatContext &ctx)
  {
    if (load.end_time < Time{0})
    {
      return format_to(
          ctx.out(), "[Load: id={}, size={} st={}]", load.id, load.size, load.send_time);
    }
    else
    {
      return format_to(
          ctx.out(),
          "[Load: id={}, size={}, st={}, dt={}]",
          load.id,
          load.size,
          load.send_time,
          load.end_time - load.send_time);
    }
  }
};
} // namespace fmt
