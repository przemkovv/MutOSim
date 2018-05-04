
#include "event_format.h"

#include <fmt/ostream.h>

namespace Simulation
{
void
format_arg(
    fmt::BasicFormatter<char> &f, const char *& /* format_str */, const EventType &type)
{
  f.writer().write([type]() {
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

void
format_arg(
    fmt::BasicFormatter<char> &f, const char *& /* format_str */, const Event &event)
{
  f.writer().write("[Event: id={}, type={}, t={}]", event.id, event.type, event.time);
}

void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const LoadServiceRequestEvent &event)
{
  format_arg(f, format_str, static_cast<const Event &>(event));
}

void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const ProduceServiceRequestEvent &event)
{
  format_arg(f, format_str, static_cast<const Event &>(event));
}

void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const LoadServiceEndEvent &event)
{
  format_arg(f, format_str, static_cast<const Event &>(event));
}
} // namespace Simulation
