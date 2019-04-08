
#pragma once
#include "config.h"

#include <experimental/source_location>
#include <fmt/color.h>
#include <fmt/format.h>

namespace fmt {
template <>
struct formatter<std::experimental::source_location>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const std::experimental::source_location &location, FormatContext &ctx)
  {
    return format_to(
        ctx.begin(), "{}:{}", location.file_name(), location.line());
  }
};
} // namespace fmt

inline const auto &
location(
    const std::experimental::source_location &loc =
        std::experimental::source_location::current())
{
  return loc;
}

template <bool enable = true, typename... Args>
void
debug_println(Args &&... args)
{
  if constexpr (enable && Config::debug_logger_enabled)
  {
    fmt::print(std::forward<Args>(args)...);
    fmt::print("\n");
  }
}
template <typename... Args>
void
debug_print(Args &&... args)
{
  if constexpr (Config::debug_logger_enabled)
  {
    fmt::print(std::forward<Args>(args)...);
  }
}
template <typename... Args>
void
print(Args &&... args)
{
  if constexpr (Config::logger_enabled)
  {
    fmt::print(std::forward<Args>(args)...);
  }
}
template <typename... Args>
void
println(Args &&... args)
{
  if constexpr (Config::logger_enabled)
  {
    fmt::print(std::forward<Args>(args)...);
    fmt::print("\n");
  }
}

template <typename... Args>
void
ASSERT(bool condition, Args &&... args)
{
  if constexpr (Config::assert_enabled)
    if (!condition)
    {
      fmt::print(std::forward<Args>(args)...);
      fmt::print("\n");
      std::abort();
    }
}
