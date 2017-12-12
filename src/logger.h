
#pragma once
#include "config.h"
#include <fmt/format.h>

template <typename... Args>
void debug_print(Args &&... args)
{
  if constexpr (Config::debug_logger_enabled) {
    fmt::print(std::forward<Args>(args)...);
  }
}
template <typename... Args>
void print(Args &&... args)
{
  if constexpr (Config::logger_enabled) {
    fmt::print(std::forward<Args>(args)...);
  }
}
