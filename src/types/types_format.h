#pragma once

#include "types.h"

#include <fmt/format.h>

namespace fmt
{
template <>
struct formatter<GroupName> {
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
struct formatter<TrafficClassId> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TrafficClassId &id, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(id));
  }
};
template <>
struct formatter<LoadId> {
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
struct formatter<SourceId> {
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
struct formatter<SourceName> {
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
struct formatter<TypesPrecision::Time<P>>
  : formatter<typename ts::underlying_type<TypesPrecision::Time<P>>> {
  template <typename FormatContext>
  auto format(const TypesPrecision::Time<P> &t, FormatContext &ctx)
  {
    return formatter<typename ts::underlying_type<TypesPrecision::Time<P>>>::format(get(t), ctx);
  }
};

template <typename P, typename Tag>
struct formatter<TypesPrecision::Capacity<P, Tag>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Capacity<P, Tag> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::Variance<P>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Variance<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::MeanRequestNumber<P>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::MeanRequestNumber<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::Peakedness<P>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Peakedness<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::MeanIntensity<P>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::MeanIntensity<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P, typename Tag>
struct formatter<TypesPrecision::Intensity<P, Tag>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Intensity<P, Tag> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P, typename Tag>
struct formatter<TypesPrecision::Size<P, Tag>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Size<P, Tag> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P>
struct formatter<TypesPrecision::Probability<P>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Probability<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

template <typename P>
struct formatter<TypesPrecision::Duration<P>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Duration<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};
template <typename P>
struct formatter<TypesPrecision::Count<P>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const TypesPrecision::Count<P> &value, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", get(value));
  }
};

} // namespace fmt

