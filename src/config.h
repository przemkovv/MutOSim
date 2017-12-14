#pragma once

namespace Config
{
constexpr bool constant_seed = true;
constexpr bool logger_enabled = true;
#if SIM_DEBUG == 1
constexpr bool debug_logger_enabled = true;
#else
constexpr bool debug_logger_enabled = false;
#endif

constexpr double epsilon = 0.0001;
} // namespace Config
