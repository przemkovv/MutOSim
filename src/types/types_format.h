#pragma once

#include "common.h"
#include "types.h"

#include <fmt/format.h>

namespace fmt {
template <>
struct formatter<GroupName>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const GroupName &name, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(name));
  }
};

template <>
struct formatter<TrafficClassId> : formatter<uuid_t>
{
  template <typename FormatContext>
  auto format(const TrafficClassId &id, FormatContext &ctx)
  {
    return formatter<uuid_t>::format(get(id), ctx);
  }
};
template <>
struct formatter<LoadId>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const LoadId &id, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(id));
  }
};
template <>
struct formatter<SourceId>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const SourceId &id, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(id));
  }
};
template <>
struct formatter<SourceName>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const SourceName &name, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(name));
  }
};
template <typename P>
struct formatter<TypesPrecision::Time_<P>>
  : formatter<typename ts::underlying_type<TypesPrecision::Time_<P>>>
{
  template <typename FormatContext>
  auto format(const TypesPrecision::Time_<P> &t, FormatContext &ctx)
  {
    return formatter<typename ts::underlying_type<TypesPrecision::Time_<P>>>::
        format(get(t), ctx);
  }
};

template <typename P, typename Tag>
struct formatter<TypesPrecision::Capacity_<P, Tag>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Capacity_<P, Tag> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::Variance_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Variance_<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::MeanRequestNumber_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::MeanRequestNumber_<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::Peakedness_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Peakedness_<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::MeanIntensity_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::MeanIntensity_<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P>
struct formatter<TypesPrecision::Intensity_<P>>
  : formatter<intensity_t<P>>
{
  template <typename FormatContext>
  auto
  format(const TypesPrecision::Intensity_<P> &value, FormatContext &ctx)
  {
    return formatter<intensity_t<P>>::format(get(value), ctx);
  }
};

template <typename P, typename Tag>
struct formatter<TypesPrecision::Size_<P, Tag>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Size_<P, Tag> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P>
struct formatter<TypesPrecision::Probability_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Probability_<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P>
struct formatter<TypesPrecision::Duration_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Duration_<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P>
struct formatter<TypesPrecision::Count_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Count_<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <>
struct formatter<highp::float_t>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const highp::float_t &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", mediump::float_t(value));
  }
};
template <>
struct formatter<highp::int_t>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const highp::int_t &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", mediump::int_t(value));
  }
};

} // namespace fmt
