
#pragma once
#include "config.h"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <source_location>

template <>
struct fmt::formatter<std::source_location>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::source_location &location, FormatContext &ctx)
  {
    return fmt::format_to(
        ctx.out(), "{}:{}", location.file_name(), location.line());
  }
};

inline const auto &
location(const std::source_location &loc = std::source_location::current())
{
  return loc;
}

template <bool enable = true, typename Format, typename... Args>
void
debug_println(Format &&format, Args &&...args)
{
  if constexpr (enable && Config::debug_logger_enabled)
  {
    fmt::vprint(format, fmt::make_format_args(args...));
    fmt::print("\n");
  }
}
template <typename Format, typename... Args>
void
debug_print(Format &&format, Args &&...args)
{
  if constexpr (Config::debug_logger_enabled)
  {
    fmt::vprint(format, fmt::make_format_args(args...));
  }
}
template <typename... Args>
constexpr void
print(std::string_view format, Args &&...args)
{
  if constexpr (Config::logger_enabled)
  {
    fmt::vprint(format, fmt::make_format_args(args...));
  }
}
template <typename... Args>
constexpr void
println(std::string_view format, Args &&...args)
{
  if constexpr (Config::logger_enabled)
  {
    fmt::vprint(format, fmt::make_format_args(args...));
    fmt::print("\n");
  }
}

template <typename Format, typename... Args>
void
ASSERT(bool condition, Format &&format, Args &&...args)
{
  if constexpr (Config::assert_enabled)
    if (!condition)
    {
      fmt::vprint(format, fmt::make_format_args(args...));
      fmt::print("\n");
      std::abort();
    }
}
