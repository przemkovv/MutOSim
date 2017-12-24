
#pragma once

#include "type_safe/strong_typedef.hpp"
#include <boost/multiprecision/mpfr.hpp>
#include <cstdint>
#include <experimental/memory>
#include <fmt/ostream.h>
#include <string_view>

#include <string>

// TODO(PW): when possible: remove this
using std::experimental::make_observer;
using std::experimental::observer_ptr;

namespace ts = type_safe;

using uuid_t = uint64_t;
using name_t = std::string;

using time_type = long double;
using duration_t = long double;
using count_t = int64_t;

// using time_type = boost::multiprecision::static_mpfr_float_100;
// using duration_t = boost::multiprecision::static_mpfr_float_100;
// using Size = int64_t;
using intensity_t = long double;
using Uuid = uuid_t;
using Name = name_t;

struct Intensity : ts::strong_typedef<Intensity, intensity_t>,
                   ts::strong_typedef_op::output_operator<Intensity> {
  using strong_typedef::strong_typedef;
  constexpr auto operator/(const Intensity &intensity) const
  {
    return ts::get(*this) / ts::get(intensity);
  }
};

struct Capacity : ts::strong_typedef<Capacity, count_t>,
                  ts::strong_typedef_op::equality_comparison<Capacity>,
                  ts::strong_typedef_op::output_operator<Capacity> {
  using strong_typedef::strong_typedef;
};

struct Size : ts::strong_typedef<Size, count_t>,
              ts::strong_typedef_op::equality_comparison<Size>,
              ts::strong_typedef_op::addition<Size>,
              ts::strong_typedef_op::output_operator<Size> {
  using strong_typedef::strong_typedef;
  constexpr Size &operator-=(const Size &s)
  {
    ts::get(*this) -= ts::get(s);
    return *this;
  }
  constexpr Size &operator+=(const Size &s)
  {
    ts::get(*this) += ts::get(s);
    return *this;
  }
  constexpr bool operator==(const Capacity &c)
  {
    return ts::get(*this) == ts::get(c);
  }
  constexpr bool operator<=(const Capacity &c)
  {
    return ts::get(*this) <= ts::get(c);
  }
  constexpr bool operator>(const Capacity &c)
  {
    return ts::get(*this) > ts::get(c);
  }
};

struct Count : ts::strong_typedef<Count, count_t>,
               ts::strong_typedef_op::equality_comparison<Count>,
               ts::strong_typedef_op::increment<Count>,
               ts::strong_typedef_op::decrement<Count>,
               ts::strong_typedef_op::subtraction<Count>,
               ts::strong_typedef_op::relational_comparison<Count>,
               ts::strong_typedef_op::output_operator<Count> {
  using strong_typedef::strong_typedef;

  constexpr auto operator*(const Intensity &intensity)
  {
    return ts::get(*this) * ts::get(intensity);
  }
};

constexpr auto operator/(const long double &x, const Count &c)
{
  return x / ts::get(c);
}
constexpr auto operator/(const Intensity &intensity, const Count &count)
{
  return Intensity{ts::get(intensity) / ts::get(count)};
}

struct Duration : ts::strong_typedef<Duration, duration_t>,
                  ts::strong_typedef_op::equality_comparison<Duration>,
                  ts::strong_typedef_op::output_operator<Duration> {
  using strong_typedef::strong_typedef;
  constexpr Duration &operator+=(const Duration &d)
  {
    ts::get(*this) += ts::get(d);
    return *this;
  }
  constexpr Duration operator+(const Duration &d) const
  {
    return Duration{ts::get(*this) + ts::get(d)};
  }
  constexpr Duration operator-(const Duration &d) const
  {
    return Duration{ts::get(*this) - ts::get(d)};
  }
  constexpr auto operator/(const Duration &duration) const
  {
    return ts::get(*this) / ts::get(duration);
  }
};

struct Time : ts::strong_typedef<Time, time_type>,
              ts::strong_typedef_op::equality_comparison<Time>,
              ts::strong_typedef_op::relational_comparison<Time>,
              ts::strong_typedef_op::output_operator<Time> {
  using strong_typedef::strong_typedef;
  explicit constexpr operator Duration() { return Duration(ts::get(*this)); }
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
                   ts::strong_typedef_op::output_operator<GroupName> {
  using strong_typedef::strong_typedef;
  GroupName operator+(std::string_view str)
  {
    return GroupName(ts::get(*this) + name_t(str));
  }
};

struct SourceName : ts::strong_typedef<SourceName, name_t>,
                    ts::strong_typedef_op::equality_comparison<SourceName>,
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

template <typename T>
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const boost::multiprecision::number<T> &number)
{
  f.writer().write("{}", number.str());
}

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
} // namespace std
