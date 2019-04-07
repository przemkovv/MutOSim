
#pragma once

#include "stream_properties.h"
#include "types/types_format.h"

#include <fmt/format.h>

namespace fmt {
template <>
struct formatter<Model::OutgoingRequestStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Model::OutgoingRequestStream &rs, FormatContext &ctx)
  {
    return format_to(
        ctx.begin(),
        "[OutgoingRequestStream] {} P_block={}, V={}, A={}, R={}, sigma^2={}, Z={}, "
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
struct formatter<Model::IncomingRequestStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Model::IncomingRequestStream &rs, FormatContext &ctx)
  {
    return format_to(
        ctx.begin(),
        "[IncomingRequestStream] {} R={}, sigma^2={}, Z={}, A={}",
        rs.tc,
        rs.mean,
        rs.variance,
        rs.peakedness,
        rs.intensity);
  }
};

template <typename T>
struct formatter<std::vector<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::vector<T> &vec, FormatContext &ctx)
  {
    format_to(ctx.out(), "S({}) [", std::size(vec));
    for (const auto &x : vec)
    {
      auto s = fmt::format("{}, ", x);
      if (s.size() > 16)
      {
        format_to(ctx.out(), "\n");
      }
      format_to(ctx.out(), s);
    }
    format_to(ctx.out(), "]");
    return ctx.out();
  }
};
} // namespace fmt
