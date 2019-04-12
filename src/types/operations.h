#pragma once

#include "common.h"

#include <type_traits>

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
inline constexpr bool is_highp_float_v =
    std::is_same_v<remove_cvref_t<T>, highp::float_t>;
template <typename T>
inline constexpr bool is_highp_int_v =
    std::is_same_v<remove_cvref_t<T>, highp::int_t>;
template <typename T>
inline constexpr bool is_highp_v = is_highp_float_v<T> || is_highp_int_v<T>;

template <typename T>
constexpr auto
add(T &&v1, T &&v2)
{
  return v1 + v2;
}
template <typename T>
constexpr auto
sub(T &&v1, T &&v2)
{
  return v1 - v2;
}
template <typename T>
constexpr auto
mul(T &&v1, T &&v2)
{
  return v1 * v2;
}
template <typename T>
constexpr auto
div(T &&v1, T &&v2)
{
  return v1 / v2;
}

template <typename T1, typename T2>
using highp_promote_t = std::conditional_t<
    is_highp_float_v<T1> || is_highp_float_v<T2>,
    highp::float_t,
    highp::int_t>;

template <
    typename T1,
    typename T2,
    typename Result = std::enable_if_t<
        !std::is_same_v<T1, T2> && is_highp_v<T1> && is_highp_v<T2>,
        highp_promote_t<T1, T2>>>
constexpr Result
add(T1 &&v1, T2 &&v2)
{
  return static_cast<Result>(v1) + static_cast<Result>(v2);
}
template <
    typename T1,
    typename T2,
    typename Result = std::enable_if_t<
        !std::is_same_v<T1, T2> && is_highp_v<T1> && is_highp_v<T2>,
        highp_promote_t<T1, T2>>>
constexpr Result
sub(T1 &&v1, T2 &&v2)
{
  return static_cast<Result>(v1) - static_cast<Result>(v2);
}
template <
    typename T1,
    typename T2,
    typename Result = std::enable_if_t<
        !std::is_same_v<T1, T2> && is_highp_v<T1> && is_highp_v<T2>,
        highp_promote_t<T1, T2>>>
constexpr Result
mul(T1 &&v1, T2 &&v2)
{
  return static_cast<Result>(v1) * static_cast<Result>(v2);
}
template <
    typename T1,
    typename T2,
    typename Result = std::enable_if_t<
        !std::is_same_v<T1, T2> && is_highp_v<T1> && is_highp_v<T2>,
        highp_promote_t<T1, T2>>>
constexpr Result
div(T1 &&v1, T2 &&v2)
{
  return static_cast<Result>(v1) / static_cast<Result>(v2);
}
