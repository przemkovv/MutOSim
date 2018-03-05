
#pragma once

#include <cstdint>
#include <string_view>
#include <type_safe/strong_typedef.hpp>

#include <fmt/ostream.h>
#include <string>

namespace ts = type_safe;

using uuid_t = uint64_t;
using name_t = std::string;

using time_type = long double;
using duration_t = long double;
using count_t = int64_t;

using intensity_t = long double;
using Uuid = uuid_t;
using Name = name_t;
using weight_t = uint64_t;
using ratio_t = long double;
using Layer = uint64_t;
using Length = uint64_t;
using OverflowPolicyName = name_t;
using threshold_t = count_t;

constexpr auto MaxPathLength = std::numeric_limits<Length>::max();
constexpr Layer MaxLayersNumber = 3;

struct Ratio : ts::strong_typedef<Ratio, ratio_t>,
               ts::strong_typedef_op::equality_comparison<Ratio>,
               ts::strong_typedef_op::multiplication<Ratio>,
               ts::strong_typedef_op::output_operator<Ratio> {
  using strong_typedef::strong_typedef;
};

struct Weight : ts::strong_typedef<Weight, weight_t>,
                ts::strong_typedef_op::equality_comparison<Weight>,
                ts::strong_typedef_op::addition<Weight>,
                ts::strong_typedef_op::output_operator<Weight> {
  using strong_typedef::strong_typedef;
  constexpr Ratio operator/(const Weight &w) const
  {
    return Ratio{static_cast<ts::underlying_type<Ratio>>(ts::get(*this)) / ts::get(w)};
  }
};

struct Capacity : ts::strong_typedef<Capacity, count_t>,
                  ts::strong_typedef_op::equality_comparison<Capacity>,
                  ts::strong_typedef_op::relational_comparison<Capacity>,
                  ts::strong_typedef_op::addition<Capacity>,
                  ts::strong_typedef_op::output_operator<Capacity> {
  using strong_typedef::strong_typedef;
};

struct Size : ts::strong_typedef<Size, count_t>,
              ts::strong_typedef_op::equality_comparison<Size>,
              ts::strong_typedef_op::relational_comparison<Size>,
              ts::strong_typedef_op::addition<Size>,
              ts::strong_typedef_op::subtraction<Size>,
              ts::strong_typedef_op::output_operator<Size> {
  using strong_typedef::strong_typedef;

  constexpr bool operator==(const Capacity &c) { return ts::get(*this) == ts::get(c); }
  constexpr bool operator<=(const Capacity &c) { return ts::get(*this) <= ts::get(c); }
  constexpr bool operator>(const Capacity &c) { return ts::get(*this) > ts::get(c); }
};

constexpr auto operator-(const Capacity &c, const Size &s)
{
  return Size{get(c) - get(s)};
}

struct Intensity : ts::strong_typedef<Intensity, intensity_t>,
                   ts::strong_typedef_op::multiplication<Intensity>,
                   ts::strong_typedef_op::addition<Intensity>,
                   ts::strong_typedef_op::relational_comparison<Intensity>,
                   ts::strong_typedef_op::output_operator<Intensity> {
  using strong_typedef::strong_typedef;
  constexpr auto operator/(const Intensity &intensity) const
  {
    return ts::get(*this) / ts::get(intensity);
  }
  constexpr auto operator/(const Size &size) const
  {
    return Intensity(ts::get(*this) / ts::get(size));
  }
  constexpr auto operator*(const Capacity &capacity) const
  {
    return Intensity(ts::get(*this) * ts::get(capacity));
  }
  constexpr auto operator*(const Ratio &ratio) const
  {
    return Intensity(ts::get(*this) * ts::get(ratio));
  }
};

struct IntensityFactor : ts::strong_typedef<IntensityFactor, intensity_t>,
                         ts::strong_typedef_op::multiplication<IntensityFactor>,
                         ts::strong_typedef_op::output_operator<IntensityFactor> {
  using strong_typedef::strong_typedef;

  constexpr auto operator*(const Intensity &intensity) const
  {
    return Intensity(ts::get(*this) * ts::get(intensity));
  }
};

struct Count : ts::strong_typedef<Count, count_t>,
               ts::strong_typedef_op::equality_comparison<Count>,
               ts::strong_typedef_op::increment<Count>,
               ts::strong_typedef_op::decrement<Count>,
               ts::strong_typedef_op::subtraction<Count>,
               ts::strong_typedef_op::addition<Count>,
               ts::strong_typedef_op::relational_comparison<Count>,
               ts::strong_typedef_op::output_operator<Count> {
  using strong_typedef::strong_typedef;
};

constexpr auto operator*(const Count &count, const Intensity &intensity)
{
  return ts::get(count) * ts::get(intensity);
}
constexpr auto operator/(const Intensity &intensity, const Count &count)
{
  return Intensity{ts::get(intensity) / ts::get(count)};
}

struct Duration : ts::strong_typedef<Duration, duration_t>,
                  ts::strong_typedef_op::equality_comparison<Duration>,
                  ts::strong_typedef_op::subtraction<Duration>,
                  ts::strong_typedef_op::addition<Duration>,
                  ts::strong_typedef_op::output_operator<Duration> {
  using strong_typedef::strong_typedef;
  constexpr auto operator/(const Duration &duration) const
  {
    return ts::get(*this) / ts::get(duration);
  }
  constexpr auto operator*(const Size &size) const
  {
    return Duration{ts::get(*this) / ts::get(size)};
  }
};

struct Time : ts::strong_typedef<Time, time_type>,
              ts::strong_typedef_op::equality_comparison<Time>,
              ts::strong_typedef_op::relational_comparison<Time>,
              ts::strong_typedef_op::output_operator<Time> {
  using strong_typedef::strong_typedef;
  explicit constexpr operator Duration() const { return Duration(ts::get(*this)); }
  constexpr Time &operator+=(const Duration &d)
  {
    ts::get(*this) += ts::get(d);
    return *this;
  }
  constexpr Time operator+(const Duration &duration) const
  {
    return Time{ts::get(*this) + ts::get(duration)};
  }
  constexpr Duration operator-(const Time &t) const
  {
    return Duration{ts::get(*this) - ts::get(t)};
  }
};

struct GroupName : ts::strong_typedef<GroupName, name_t>,
                   ts::strong_typedef_op::equality_comparison<GroupName>,
                   ts::strong_typedef_op::relational_comparison<GroupName>,
                   ts::strong_typedef_op::output_operator<GroupName> {
  using strong_typedef::strong_typedef;
  GroupName operator+(std::string_view str)
  {
    return GroupName(ts::get(*this) + name_t(str));
  }
};

struct SourceName : ts::strong_typedef<SourceName, name_t>,
                    ts::strong_typedef_op::equality_comparison<SourceName>,
                    ts::strong_typedef_op::relational_comparison<SourceName>,
                    ts::strong_typedef_op::output_operator<SourceName> {
  using strong_typedef::strong_typedef;
};

struct GroupId : ts::strong_typedef<GroupId, uuid_t>,
                 ts::strong_typedef_op::equality_comparison<GroupId>,
                 ts::strong_typedef_op::output_operator<GroupId> {
  using strong_typedef::strong_typedef;
};
struct SourceId : ts::strong_typedef<SourceId, uuid_t>,
                  ts::strong_typedef_op::equality_comparison<SourceId>,
                  ts::strong_typedef_op::output_operator<SourceId> {
  using strong_typedef::strong_typedef;
};
struct LoadId : ts::strong_typedef<LoadId, uuid_t>,
                ts::strong_typedef_op::equality_comparison<LoadId>,
                ts::strong_typedef_op::output_operator<LoadId> {
  using strong_typedef::strong_typedef;
};

struct TrafficClassId : ts::strong_typedef<TrafficClassId, uuid_t>,
                        ts::strong_typedef_op::equality_comparison<TrafficClassId>,
                        ts::strong_typedef_op::relational_comparison<TrafficClassId>,
                        ts::strong_typedef_op::output_operator<TrafficClassId> {
  using strong_typedef::strong_typedef;
};

namespace std
{
/// Hash specialization for [GroupName].
template <>
struct hash<GroupName> {
  std::size_t operator()(const GroupName &i) const noexcept
  {
    using T = ts::underlying_type<GroupName>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [SourceName].
template <>
struct hash<SourceName> {
  std::size_t operator()(const SourceName &i) const noexcept
  {
    using T = ts::underlying_type<SourceName>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [TrafficClassId].
template <>
struct hash<TrafficClassId> {
  std::size_t operator()(const TrafficClassId &i) const noexcept
  {
    using T = ts::underlying_type<TrafficClassId>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [LoadId].
template <>
struct hash<LoadId> {
  std::size_t operator()(const LoadId &i) const noexcept
  {
    using T = ts::underlying_type<LoadId>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [SourceId].
template <>
struct hash<SourceId> {
  std::size_t operator()(const SourceId &i) const noexcept
  {
    using T = ts::underlying_type<SourceId>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [Size].
template <>
struct hash<Size> {
  std::size_t operator()(const Size &i) const noexcept
  {
    using T = ts::underlying_type<Size>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [Capacity].
template <>
struct hash<Capacity> {
  std::size_t operator()(const Capacity &i) const noexcept
  {
    using T = ts::underlying_type<Capacity>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
} // namespace std
