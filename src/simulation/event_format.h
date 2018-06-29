
#pragma once
#include "event.h"

#include <fmt/format.h>

namespace Simulation
{
} // namespace Simulation

namespace fmt
{
template <>
struct formatter<Simulation::EventType> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::EventType &type, FormatContext &ctx)
  {
    return format_to(ctx.begin(), [type]() {
      switch (type) {
      case Simulation::EventType::LoadServiceRequest:
        return "LoadServiceRequest";
      case Simulation::EventType::LoadServiceEnd:
        return "LoadServiceEnd";
      case Simulation::EventType::LoadProduce:
        return "LoadProduce";
      case Simulation::EventType::None:
        return "None";
      }
    }());
  }
};

template <>
struct formatter<Simulation::Event> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::Event &event, FormatContext &ctx)
  {
    return format_to(
        ctx.begin(), "[Event: id={}, type={}, t={}]", event.id, event.type, event.time);
  }
  // format_arg(f, format_str, static_cast<const Event &>(event));
};

} // namespace fmt
