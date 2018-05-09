
#include "event.h"

#include <fmt/format.h>

namespace Simulation
{
void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const LoadServiceRequestEvent &event);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const LoadServiceEndEvent &event);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const ProduceServiceRequestEvent &event);

void
format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const Event &event);

void format_arg(
    fmt::BasicFormatter<char> &f, const char *& /* format_str */, const EventType &type);
} // namespace Simulation
