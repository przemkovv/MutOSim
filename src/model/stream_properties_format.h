
#pragma once

#include "stream_properties.h"
#include "types/types_format.h"

#include <fmt/format.h>
#include <map>

template <>
struct fmt::formatter<Model::OutgoingRequestStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Model::OutgoingRequestStream &rs, FormatContext &ctx)
  {
    return fmt::format_to(
        ctx.out(),
        "[OutgoingRequestStream] {} P_block={}, V_fict={}, A={}, R={}, "
        "sigma^2={}, Z={}, "
        "Y={}",
        rs.tc,
        rs.blocking_probability,
        rs.fictitous_capacity,
        rs.intensity,
        rs.mean,
        rs.variance,
        rs.peakedness,
        rs.mean_request_number);
  }
};
template <>
struct fmt::formatter<Model::IncomingRequestStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Model::IncomingRequestStream &rs, FormatContext &ctx)
  {
    return fmt::format_to(
        ctx.out(),
        "[IncomingRequestStream] {} R={}, sigma^2={}, Z={}, A={}",
        rs.tc,
        rs.mean,
        rs.variance,
        rs.peakedness,
        rs.intensity);
  }
};

template <typename K, typename V>
struct fmt::formatter<std::map<K, V>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::map<K, V> &m, FormatContext &ctx)
  {
    fmt::format_to(ctx.out(), "S({}) [", std::size(m));
    for (const auto &[k, v] : m)
    {
      auto s = fmt::format("('{}': '{}'), ", k, v);
      if (s.size() > 16)
      {
        fmt::format_to(ctx.out(), "\n");
      }
      fmt::format_to(ctx.out(), s);
    }
    fmt::format_to(ctx.out(), "]");
    return ctx.out();
  }
};

template <typename T>
struct fmt::formatter<std::vector<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::vector<T> &vec, FormatContext &ctx) const
  {
    fmt::format_to(ctx.out(), "S({}) [", std::size(vec));
    for (const auto &x : vec)
    {
      auto s = fmt::format("{}, ", x);
      if (s.size() > 16)
      {
        fmt::format_to(ctx.out(), "\n");
      }
      fmt::format_to(ctx.out(), "{}", s);
    }
    fmt::format_to(ctx.out(), "]");
    return ctx.out();
  }
};
