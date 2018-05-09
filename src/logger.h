
#pragma once
#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

template <typename... Args>
void
debug_println(Args &&... args)
{
  if constexpr (Config::debug_logger_enabled) {
    fmt::print(std::forward<Args>(args)...);
    fmt::print("\n");
  }
}
template <typename... Args>
void
debug_print(Args &&... args)
{
  if constexpr (Config::debug_logger_enabled) {
    fmt::print(std::forward<Args>(args)...);
  }
}
template <typename... Args>
void
print(Args &&... args)
{
  if constexpr (Config::logger_enabled) {
    fmt::print(std::forward<Args>(args)...);
  }
}
template <typename... Args>
void
println(Args &&... args)
{
  if constexpr (Config::logger_enabled) {
    fmt::print(std::forward<Args>(args)...);
    fmt::print("\n");
  }
}

template <typename... Args>
void
ASSERT(bool condition, Args &&... args)
{
  if constexpr (Config::assert_enabled)
    if (!condition) {
      fmt::print(std::forward<Args>(args)...);
      fmt::print("\n");
      std::abort();
    }
}
