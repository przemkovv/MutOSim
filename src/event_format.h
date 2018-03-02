
#include "event.h"

#include <fmt/format.h>

namespace fmt
{
template <>
struct formatter<EventType> {
  template <typename ParseContext>
  auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const EventType &type, FormatContext &ctx)
  {
    fmt::format_to(ctx.begin(), [type]() {
      switch (type) {
      case EventType::LoadServiceRequest:
        return "LoadServiceRequest";
      case EventType::LoadServiceEnd:
        return "LoadServiceEnd";
      case EventType::LoadProduce:
        return "LoadProduce";
      case EventType::None:
        return "None";
      }
    }());
  }
};

template <>
struct formatter<Event> {
  template <typename ParseContext>
  auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Event &event, FormatContext &ctx)
  {
    fmt::format_to(ctx.begin(), "[Event: id={}, type={}, t={}]", event.id, event.type,
                   event.time);
  }
};

} // namespace fmt
