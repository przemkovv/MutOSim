#pragma once

namespace Config
{
constexpr bool constant_seed = false;
constexpr bool logger_enabled = true;
constexpr bool assert_enabled = true;
#if SIM_DEBUG == 1
constexpr bool debug_logger_enabled = true;
#else
constexpr bool debug_logger_enabled = false;
#endif

} // namespace Config
