#pragma once

namespace Config {
inline constexpr bool constant_seed = false;
inline constexpr bool logger_enabled = true;
inline constexpr bool assert_enabled = true;
#if SIM_DEBUG == 1
inline constexpr bool debug_mpfr = true;
inline constexpr bool debug_logger_enabled = true;
#else
inline constexpr bool debug_mpfr = false;
inline constexpr bool debug_logger_enabled = false;
#endif

} // namespace Config
