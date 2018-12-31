
#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_safe/strong_typedef.hpp>
#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/gmp.hpp>

namespace ts = type_safe;

constexpr long double epsilon = 0.00000001L;

using uuid_t = uint64_t;
using name_t = std::string;
using OverflowPolicyName = name_t;

using Uuid = uuid_t;
using Name = name_t;
using Layer = uint64_t;
using Length = uint64_t;

struct mediump {
 using float_t = long double;
 using  int_t = int64_t;
};
struct highp {
  using float_t = boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<100>>;
  using int_t = boost::multiprecision::mpz_int;
};

template <typename Precision = mediump>
using time_type = typename Precision::float_t;

template <typename Precision = mediump>
using duration_t = typename Precision::float_t;
template <typename Precision = mediump>
using count_t = typename Precision::int_t;
template <typename Precision = mediump>
using count_float_t = typename Precision::float_t;

template <typename Precision = mediump>
using stat_t = typename Precision::float_t;
template <typename Precision = mediump>
using probability_t = typename Precision::float_t;
template <typename Precision = mediump>
using intensity_t = typename Precision::float_t;
template <typename Precision = mediump>
using weight_t = typename Precision::int_t;
template <typename Precision = mediump>
using weight_float_t = typename Precision::float_t;
template <typename Precision = mediump>
using ratio_t = typename Precision::float_t;
template <typename Precision = mediump>
using threshold_t = count_t<Precision>;

constexpr auto MaxPathLength = std::numeric_limits<Length>::max();
constexpr Layer MaxLayersNumber = 3;

struct Ratio : ts::strong_typedef<Ratio, ratio_t<>>,
               ts::strong_typedef_op::equality_comparison<Ratio>,
               ts::strong_typedef_op::multiplication<Ratio>,
               ts::strong_typedef_op::output_operator<Ratio> {
  using strong_typedef::strong_typedef;
};

struct InvWeightF : ts::strong_typedef<InvWeightF, weight_float_t<>>,
                    ts::strong_typedef_op::equality_comparison<InvWeightF>,
                    ts::strong_typedef_op::addition<InvWeightF>,
                    ts::strong_typedef_op::output_operator<InvWeightF> {
  using strong_typedef::strong_typedef;
};

struct WeightF : ts::strong_typedef<WeightF, weight_float_t<>>,
                 ts::strong_typedef_op::equality_comparison<WeightF>,
                 ts::strong_typedef_op::addition<WeightF>,
                 ts::strong_typedef_op::output_operator<WeightF> {
  using strong_typedef::strong_typedef;
  constexpr auto invert() { return InvWeightF{weight_float_t<>{1} / get(*this)}; }
};
struct Weight : ts::strong_typedef<Weight, weight_t<>>,
                ts::strong_typedef_op::equality_comparison<Weight>,
                ts::strong_typedef_op::addition<Weight>,
                ts::strong_typedef_op::output_operator<Weight> {
  using strong_typedef::strong_typedef;
  constexpr Ratio operator/(const Weight &w) const
  {
    return Ratio{static_cast<ts::underlying_type<Ratio>>(ts::get(*this)) / ts::get(w)};
  }
};

struct Capacity : ts::strong_typedef<Capacity, count_t<>>,
                  ts::strong_typedef_op::equality_comparison<Capacity>,
                  ts::strong_typedef_op::relational_comparison<Capacity>,
                  ts::strong_typedef_op::addition<Capacity>,
                  ts::strong_typedef_op::subtraction<Capacity>,
                  ts::strong_typedef_op::increment<Capacity>,
                  ts::strong_typedef_op::output_operator<Capacity> {
  using strong_typedef::strong_typedef;
  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }
};
struct CapacityF : ts::strong_typedef<CapacityF, count_float_t<>>,
                   ts::strong_typedef_op::equality_comparison<CapacityF>,
                   ts::strong_typedef_op::relational_comparison<CapacityF>,
                   ts::strong_typedef_op::addition<CapacityF>,
                   ts::strong_typedef_op::subtraction<CapacityF>,
                   ts::strong_typedef_op::increment<CapacityF>,
                   ts::strong_typedef_op::output_operator<CapacityF> {
  using strong_typedef::strong_typedef;
  constexpr CapacityF(const Capacity &c) : CapacityF(get(c)) {}

  explicit operator Capacity() { return Capacity{static_cast<count_t<>>(get(*this))}; }
  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }
};

struct Size : ts::strong_typedef<Size, count_t<>>,
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

struct SizeF : ts::strong_typedef<SizeF, count_float_t<>>,
               ts::strong_typedef_op::equality_comparison<SizeF>,
               ts::strong_typedef_op::relational_comparison<SizeF>,
               ts::strong_typedef_op::addition<SizeF>,
               ts::strong_typedef_op::subtraction<SizeF>,
               ts::strong_typedef_op::output_operator<SizeF> {
  using strong_typedef::strong_typedef;

  constexpr SizeF(const Size &c) : SizeF(get(c)) {}
  explicit operator Size() { return Size{static_cast<count_t<>>(std::ceil(get(*this)))}; }
};
constexpr auto
operator-(const Capacity &c, const Size &s)
{
  return Capacity{get(c) - get(s)};
}
constexpr auto
operator+(const CapacityF &c, const Size &s)
{
  return CapacityF{get(c) + get(s)};
}
constexpr auto
operator-(const CapacityF &c, const SizeF &s)
{
  return CapacityF{get(c) - get(s)};
}
constexpr auto
operator-(const Capacity &c, const SizeF &s)
{
  return CapacityF{get(c) - get(s)};
}
constexpr auto &
operator+=(CapacityF &c, const SizeF &s)
{
  get(c) = get(c) + get(s);
  return c;
}
constexpr auto &
operator+=(Capacity &c, const Size &s)
{
  get(c) = get(c) + get(s);
  return c;
}
constexpr auto
operator+(const Capacity &c, const Size &s)
{
  return Capacity{get(c) + get(s)};
}
constexpr auto
operator>=(const Capacity &c, const Size &s)
{
  return get(c) >= get(s);
}
struct IntensitySize : ts::strong_typedef<IntensitySize, intensity_t<>>,
                       ts::strong_typedef_op::multiplication<IntensitySize>,
                       ts::strong_typedef_op::output_operator<IntensitySize> {
  using strong_typedef::strong_typedef;
};

struct Intensity : ts::strong_typedef<Intensity, intensity_t<>>,
                   ts::strong_typedef_op::multiplication<Intensity>,
                   ts::strong_typedef_op::addition<Intensity>,
                   ts::strong_typedef_op::relational_comparison<Intensity>,
                   ts::strong_typedef_op::output_operator<Intensity> {
  using strong_typedef::strong_typedef;
  constexpr auto operator/(const Intensity &intensity) const
  {
    return Intensity{ts::get(*this) / ts::get(intensity)};
  }
  constexpr auto operator/(const Size &size) const
  {
    return Intensity(ts::get(*this) / ts::get(size));
  }
  constexpr auto operator*(const SizeF &size) const
  {
    return IntensitySize(ts::get(*this) * ts::get(size));
  }
  constexpr auto operator*(const Size &size) const
  {
    return IntensitySize(ts::get(*this) * ts::get(size));
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

constexpr bool
operator==(const Intensity &intensity1, const Intensity &intensity2)
{
  return (get(intensity1) + epsilon) > get(intensity2) &&
         (get(intensity1) - epsilon) < get(intensity2);
}

constexpr auto
operator/(const IntensitySize &intensity, const Size &size)
{
  return Intensity(ts::get(intensity) / ts::get(size));
}
constexpr auto
operator/(const IntensitySize &intensity, const Capacity &size)
{
  return Intensity(ts::get(intensity) / ts::get(size));
}
struct Probability : ts::strong_typedef<Probability, probability_t<>>,
                     ts::strong_typedef_op::multiplication<Probability>,
                     ts::strong_typedef_op::division<Probability>,
                     ts::strong_typedef_op::addition<Probability>,
                     ts::strong_typedef_op::subtraction<Probability>,
                     ts::strong_typedef_op::relational_comparison<Probability>,
                     ts::strong_typedef_op::equality_comparison<Probability>,
                     ts::strong_typedef_op::output_operator<Probability> {
  using strong_typedef::strong_typedef;
  constexpr Probability &operator/=(const Capacity &capacity)
  {
    ts::get(*this) /= ts::get(capacity);
    return *this;
  }
  constexpr Probability opposite() { return Probability{1 - get(*this)}; }

  explicit constexpr Probability(const Ratio &ratio) { get(*this) = get(ratio); }
};

constexpr auto
operator/(const Probability &probability, const Capacity &capacity)
{
  return Probability{get(probability) / get(capacity)};
}

constexpr auto operator*(const IntensitySize &intensity, const Probability &probability)
{
  return Probability{get(intensity) * get(probability)};
}

struct IntensityFactor : ts::strong_typedef<IntensityFactor, intensity_t<>>,
                         ts::strong_typedef_op::multiplication<IntensityFactor>,
                         ts::strong_typedef_op::output_operator<IntensityFactor> {
  using strong_typedef::strong_typedef;

  constexpr auto operator*(const Intensity &intensity) const
  {
    return Intensity(ts::get(*this) * ts::get(intensity));
  }
};

constexpr auto
operator/(const Size &size, const IntensityFactor &intensity_factor)
{
  return Size{static_cast<count_t<>>(get(size) / get(intensity_factor))};
}

struct Count : ts::strong_typedef<Count, count_t<>>,
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
constexpr auto
operator/(const Intensity &intensity, const Count &count)
{
  return Intensity{ts::get(intensity) / ts::get(count)};
}
constexpr auto
operator/(const Count &c1, const Count &c2)
{
  return Ratio{static_cast<ratio_t<>>(ts::get(c1)) / static_cast<ratio_t<>>(ts::get(c2))};
}

struct Duration : ts::strong_typedef<Duration, duration_t<>>,
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
struct Variance : ts::strong_typedef<Variance, stat_t<>>,
                  ts::strong_typedef_op::addition<Variance>,
                  ts::strong_typedef_op::output_operator<Variance> {
  using strong_typedef::strong_typedef;
  explicit Variance(const Intensity &intensity) : Variance(get(intensity)) {}
};
struct MeanRequestNumber : ts::strong_typedef<MeanRequestNumber, intensity_t<>>,
                           ts::strong_typedef_op::addition<MeanRequestNumber>,
                           ts::strong_typedef_op::output_operator<MeanRequestNumber> {
  using strong_typedef::strong_typedef;
};
struct MeanIntensity : ts::strong_typedef<MeanIntensity, intensity_t<>>,
                       ts::strong_typedef_op::addition<MeanIntensity>,
                       ts::strong_typedef_op::output_operator<MeanIntensity> {
  using strong_typedef::strong_typedef;
  explicit operator MeanRequestNumber() { return MeanRequestNumber{get(*this)}; }
  explicit MeanIntensity(const Intensity &intensity) : MeanIntensity(get(intensity)) {}
};

struct Peakedness : ts::strong_typedef<Peakedness, stat_t<>>,
                    ts::strong_typedef_op::addition<Peakedness>,
                    ts::strong_typedef_op::relational_comparison<Peakedness>,
                    ts::strong_typedef_op::output_operator<Peakedness> {
  using strong_typedef::strong_typedef;
};

constexpr auto operator*(const Variance &variance, const WeightF &weight)
{
  return Peakedness{get(variance) * get(weight)};
}
constexpr auto
operator/(const Variance &variance, const MeanIntensity &mean)
{
  return Peakedness{get(variance) / get(mean)};
}
constexpr auto
operator/(const MeanIntensity &mean, const Peakedness &peakedness)
{
  return Intensity{get(mean) / get(peakedness)};
}

constexpr auto operator*(const Intensity &intensity, const Probability &probability)
{
  return MeanIntensity{get(intensity) * get(probability)};
}

constexpr auto
operator/(const Capacity &capacity, const Peakedness &peakedness)
{
  return CapacityF{static_cast<stat_t<>>(get(capacity)) / get(peakedness)};
}
constexpr auto operator*(const MeanIntensity &mean, const Size &size)
{
  return WeightF{get(mean) * static_cast<weight_float_t<>>(get(size))};
}
constexpr auto operator*(const MeanRequestNumber &mean, const Size &size)
{
  return CapacityF{get(mean) * static_cast<intensity_t<>>(get(size))};
}
constexpr auto operator*(const MeanRequestNumber &mean, const SizeF &size)
{
  return CapacityF{get(mean) * static_cast<intensity_t<>>(get(size))};
}
constexpr auto operator*(const Size &size, const InvWeightF &inv_weight)
{
  return WeightF{static_cast<stat_t<>>(get(size)) * get(inv_weight)};
}

struct SizeRescale : ts::strong_typedef<SizeRescale, intensity_t<>>,
                     ts::strong_typedef_op::output_operator<SizeRescale> {
  using strong_typedef::strong_typedef;
  explicit SizeRescale(const Peakedness &peakedness) : SizeRescale(get(peakedness)) {}
};

constexpr auto operator*(const SizeRescale &rescale, const Size &size)
{
  return SizeF{get(rescale) * get(size)};
}
constexpr auto operator*(const SizeRescale &rescale, const SizeF &size)
{
  return SizeF{get(rescale) * get(size)};
}

struct Time : ts::strong_typedef<Time, time_type<>>,
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
