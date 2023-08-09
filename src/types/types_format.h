#pragma once

#include "common.h"
#include "types.h"

#include <fmt/format.h>

template <>
struct fmt::formatter<GroupName>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const GroupName &name, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(name));
  }
};

template <>
struct fmt::formatter<TrafficClassId> : formatter<uuid_t>
{
  template <typename FormatContext>
  auto format(const TrafficClassId &id, FormatContext &ctx)
  {
    return fmt::formatter<uuid_t>::format(get(id), ctx);
  }
};
template <>
struct fmt::formatter<LoadId>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const LoadId &id, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(id));
  }
};
template <>
struct fmt::formatter<SourceId> : formatter<typename SourceId::value_type>
{
  template <typename FormatContext>
  auto format(const SourceId &id, FormatContext &ctx) const
  {
    return fmt::formatter<typename SourceId::value_type>::format(get(id), ctx);
  }
};
template <>
struct fmt::formatter<SourceName>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const SourceName &name, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(name));
  }
};
template <typename P>
struct fmt::formatter<TypesPrecision::Time_<P>>
  : formatter<typename ts::underlying_type<TypesPrecision::Time_<P>>>
{
  template <typename FormatContext>
  auto format(const TypesPrecision::Time_<P> &t, FormatContext &ctx) const
  {
    return fmt::formatter<typename ts::underlying_type<
        TypesPrecision::Time_<P>>>::format(get(t), ctx);
  }
};

template <typename P, typename Tag>
struct fmt::formatter<TypesPrecision::Capacity_<P, Tag>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Capacity_<P, Tag> &value, FormatContext &ctx)
      const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};
template <typename P>
struct fmt::formatter<TypesPrecision::Variance_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Variance_<P> &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};
template <typename P>
struct fmt::formatter<TypesPrecision::MeanRequestNumber_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::MeanRequestNumber_<P> &value, FormatContext &ctx)
      const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};
template <typename P>
struct fmt::formatter<TypesPrecision::Peakedness_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Peakedness_<P> &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};
template <typename P>
struct fmt::formatter<TypesPrecision::MeanIntensity_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::MeanIntensity_<P> &value, FormatContext &ctx)
      const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};

template <typename P, typename Tag>
struct fmt::formatter<TypesPrecision::Weight_<P, Tag>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Weight_<P, Tag> &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};

template <typename P>
struct fmt::formatter<TypesPrecision::Ratio_<P>> : formatter<ratio_t<P>>
{
  template <typename FormatContext>
  auto format(const TypesPrecision::Ratio_<P> &value, FormatContext &ctx) const
  {
    return fmt::formatter<ratio_t<P>>::format(get(value), ctx);
  }
};

template <typename P>
struct fmt::formatter<TypesPrecision::Intensity_<P>> : formatter<intensity_t<P>>
{
  template <typename FormatContext>
  auto
  format(const TypesPrecision::Intensity_<P> &value, FormatContext &ctx) const
  {
    return fmt::formatter<intensity_t<P>>::format(get(value), ctx);
  }
};

template <typename P, typename Tag>
struct fmt::formatter<TypesPrecision::Size_<P, Tag>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Size_<P, Tag> &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};

template <typename P>
struct fmt::formatter<TypesPrecision::Probability_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Probability_<P> &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};

template <typename P>
struct fmt::formatter<TypesPrecision::Duration_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const TypesPrecision::Duration_<P> &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};

template <typename P>
struct fmt::formatter<TypesPrecision::Count_<P>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Count_<P> &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", get(value));
  }
};
template <>
struct fmt::formatter<highp::float_t>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const highp::float_t &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", mediump::float_t(value));
  }
};
template <>
struct fmt::formatter<highp::int_t>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const highp::int_t &value, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", mediump::int_t(value));
  }
};
