
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

namespace Types {
template <typename Prec = mediump>
struct Ratio : ts::strong_typedef<Ratio<Prec>, ratio_t<Prec>>,
               ts::strong_typedef_op::equality_comparison<Ratio<Prec>>,
               ts::strong_typedef_op::multiplication<Ratio<Prec>>,
               ts::strong_typedef_op::output_operator<Ratio<Prec>> {
  using ts::strong_typedef<Ratio<Prec>, ratio_t<Prec>>::strong_typedef;
};

template <typename Prec = mediump>
struct InvWeightF : ts::strong_typedef<InvWeightF<Prec>, weight_float_t<Prec>>,
                    ts::strong_typedef_op::equality_comparison<InvWeightF<Prec>>,
                    ts::strong_typedef_op::addition<InvWeightF<Prec>>,
                    ts::strong_typedef_op::output_operator<InvWeightF<Prec>> {
  using ts::strong_typedef<InvWeightF<Prec>, weight_float_t<Prec>>::strong_typedef;
};

template <typename Prec = mediump>
struct WeightF : ts::strong_typedef<WeightF<Prec>, weight_float_t<Prec>>,
                 ts::strong_typedef_op::equality_comparison<WeightF<Prec>>,
                 ts::strong_typedef_op::addition<WeightF<Prec>>,
                 ts::strong_typedef_op::output_operator<WeightF<Prec>> {
  using ts::strong_typedef<WeightF<Prec>, weight_float_t<Prec>>::strong_typedef;
  constexpr auto invert() { return InvWeightF<Prec>{weight_float_t<Prec>{1} / get(*this)}; }
};
template <typename Prec = mediump>
struct Weight : ts::strong_typedef<Weight<Prec>, weight_t<Prec>>,
                ts::strong_typedef_op::equality_comparison<Weight<Prec>>,
                ts::strong_typedef_op::addition<Weight<Prec>>,
                ts::strong_typedef_op::output_operator<Weight<Prec>> {
  using ts::strong_typedef<Weight<Prec>, weight_t<Prec>>::strong_typedef;
  constexpr auto operator/(const Weight &w) const
  {
    return Ratio<Prec>{static_cast<ts::underlying_type<Ratio<Prec>>>(ts::get(*this)) / ts::get(w)};
  }
};

template <typename Prec = mediump>
struct Capacity : ts::strong_typedef<Capacity<Prec>, count_t<Prec>>,
                  ts::strong_typedef_op::equality_comparison<Capacity<Prec>>,
                  ts::strong_typedef_op::relational_comparison<Capacity<Prec>>,
                  ts::strong_typedef_op::addition<Capacity<Prec>>,
                  ts::strong_typedef_op::subtraction<Capacity<Prec>>,
                  ts::strong_typedef_op::increment<Capacity<Prec>>,
                  ts::strong_typedef_op::output_operator<Capacity<Prec>> {
  using ts::strong_typedef<Capacity<Prec>, count_t<Prec>>::strong_typedef;
  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }
};
template <typename Prec = mediump>
struct CapacityF : ts::strong_typedef<CapacityF<Prec>, count_float_t<Prec>>,
                   ts::strong_typedef_op::equality_comparison<CapacityF<Prec>>,
                   ts::strong_typedef_op::relational_comparison<CapacityF<Prec>>,
                   ts::strong_typedef_op::addition<CapacityF<Prec>>,
                   ts::strong_typedef_op::subtraction<CapacityF<Prec>>,
                   ts::strong_typedef_op::increment<CapacityF<Prec>>,
                   ts::strong_typedef_op::output_operator<CapacityF<Prec>> {
  using ts::strong_typedef<CapacityF<Prec>, count_float_t<Prec>>::strong_typedef;
  constexpr CapacityF(const Capacity<Prec> &c) : CapacityF<Prec>(get(c)) {}

  explicit operator Capacity<Prec>() { return Capacity<Prec>{static_cast<count_t<Prec>>(get(*this))}; }
  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }
};

template <typename Prec = mediump>
struct Size : ts::strong_typedef<Size<Prec>, count_t<Prec>>,
              ts::strong_typedef_op::equality_comparison<Size<Prec>>,
              ts::strong_typedef_op::relational_comparison<Size<Prec>>,
              ts::strong_typedef_op::addition<Size<Prec>>,
              ts::strong_typedef_op::subtraction<Size<Prec>>,
              ts::strong_typedef_op::output_operator<Size<Prec>> {
  using ts::strong_typedef<Size<Prec>, count_t<Prec>>::strong_typedef;

  constexpr bool operator==(const Capacity<Prec> &c) { return ts::get(*this) == ts::get(c); }
  constexpr bool operator<=(const Capacity<Prec> &c) { return ts::get(*this) <= ts::get(c); }
  constexpr bool operator>(const Capacity<Prec> &c) { return ts::get(*this) > ts::get(c); }
};

template <typename Prec = mediump>
struct SizeF : ts::strong_typedef<SizeF<Prec>, count_float_t<Prec>>,
               ts::strong_typedef_op::equality_comparison<SizeF<Prec>>,
               ts::strong_typedef_op::relational_comparison<SizeF<Prec>>,
               ts::strong_typedef_op::addition<SizeF<Prec>>,
               ts::strong_typedef_op::subtraction<SizeF<Prec>>,
               ts::strong_typedef_op::output_operator<SizeF<Prec>> {
  using ts::strong_typedef<SizeF<Prec>, count_float_t<Prec>>::strong_typedef;

  constexpr SizeF(const Size<Prec> &c) : SizeF(get(c)) {}
  explicit operator Size<Prec>() { return Size<Prec>{static_cast<count_t<Prec>>(std::ceil(get(*this)))}; }
};
template <typename Prec = mediump>
constexpr auto
operator-(const Capacity<Prec> &c, const Size<Prec> &s)
{
  return Capacity<Prec>{get(c) - get(s)};
}
template <typename Prec = mediump>
constexpr auto
operator-(const CapacityF<Prec> &c, const Size<Prec> &s)
{
  return CapacityF<Prec>{get(c) - get(s)};
}
template <typename Prec = mediump>
constexpr auto
operator+(const CapacityF<Prec> &c, const Size<Prec> &s)
{
  return CapacityF<Prec>{get(c) + get(s)};
}
template <typename Prec = mediump>
constexpr auto
operator-(const CapacityF<Prec> &c, const SizeF<Prec> &s)
{
  return CapacityF<Prec>{get(c) - get(s)};
}
template <typename Prec = mediump>
constexpr auto
operator-(const Capacity<Prec> &c, const SizeF<Prec> &s)
{
  return CapacityF<Prec>{get(c) - get(s)};
}
template <typename Prec = mediump>
constexpr auto &
operator+=(CapacityF<Prec> &c, const SizeF<Prec> &s)
{
  get(c) = get(c) + get(s);
  return c;
}
template <typename Prec = mediump>
constexpr auto &
operator+=(Capacity<Prec> &c, const Size<Prec> &s)
{
  get(c) = get(c) + get(s);
  return c;
}
template <typename Prec = mediump>
constexpr auto
operator+(const Capacity<Prec> &c, const Size<Prec> &s)
{
  return Capacity<Prec>{get(c) + get(s)};
}
template <typename Prec = mediump>
constexpr auto
operator>=(const Capacity<Prec> &c, const Size<Prec> &s)
{
  return get(c) >= get(s);
}
template <typename Prec = mediump>
struct IntensitySize : ts::strong_typedef<IntensitySize<Prec>, intensity_t<Prec>>,
                       ts::strong_typedef_op::multiplication<IntensitySize<Prec>>,
                       ts::strong_typedef_op::output_operator<IntensitySize<Prec>> {
  using ts::strong_typedef<IntensitySize<Prec>, intensity_t<Prec>>::strong_typedef;
};

template <typename Prec = mediump>
struct Intensity : ts::strong_typedef<Intensity<Prec>, intensity_t<Prec>>,
                   ts::strong_typedef_op::multiplication<Intensity<Prec>>,
                   ts::strong_typedef_op::addition<Intensity<Prec>>,
                   ts::strong_typedef_op::relational_comparison<Intensity<Prec>>,
                   ts::strong_typedef_op::output_operator<Intensity<Prec>> {
  using ts::strong_typedef<Intensity<Prec>, intensity_t<Prec>>::strong_typedef;
  constexpr auto operator/(const Intensity<Prec> &intensity) const
  {
    return Intensity<Prec>{ts::get(*this) / ts::get(intensity)};
  }
  constexpr auto operator/(const Size<Prec> &size) const
  {
    return Intensity<Prec>(ts::get(*this) / ts::get(size));
  }
  constexpr auto operator*(const SizeF<Prec> &size) const
  {
    return IntensitySize<Prec>(ts::get(*this) * ts::get(size));
  }
  constexpr auto operator*(const Size<Prec> &size) const
  {
    return IntensitySize<Prec>(ts::get(*this) * ts::get(size));
  }
  constexpr auto operator*(const Capacity<Prec> &capacity) const
  {
    return Intensity<Prec>(ts::get(*this) * ts::get(capacity));
  }
  constexpr auto operator*(const Ratio<Prec> &ratio) const
  {
    return Intensity<Prec>(ts::get(*this) * ts::get(ratio));
  }
};

template <typename Prec = mediump>
constexpr bool
operator==(const Intensity<Prec> &intensity1, const Intensity<Prec> &intensity2)
{
  return (get(intensity1) + epsilon) > get(intensity2) &&
         (get(intensity1) - epsilon) < get(intensity2);
}

template <typename Prec = mediump>
constexpr auto
operator/(const IntensitySize<Prec> &intensity, const Size<Prec> &size)
{
  return Intensity<Prec>(ts::get(intensity) / ts::get(size));
}
template <typename Prec = mediump>
constexpr auto
operator/(const IntensitySize<Prec> &intensity, const Capacity<Prec> &size)
{
  return Intensity<Prec>(ts::get(intensity) / ts::get(size));
}
template <typename Prec = mediump>
struct Probability : ts::strong_typedef<Probability<Prec>, probability_t<Prec>>,
                     ts::strong_typedef_op::multiplication<Probability<Prec>>,
                     ts::strong_typedef_op::division<Probability<Prec>>,
                     ts::strong_typedef_op::addition<Probability<Prec>>,
                     ts::strong_typedef_op::subtraction<Probability<Prec>>,
                     ts::strong_typedef_op::relational_comparison<Probability<Prec>>,
                     ts::strong_typedef_op::equality_comparison<Probability<Prec>>,
                     ts::strong_typedef_op::output_operator<Probability<Prec>> {
  using ts::strong_typedef<Probability<Prec>, probability_t<Prec>>::strong_typedef;
  constexpr Probability<Prec> &operator/=(const Capacity<Prec> &capacity)
  {
    ts::get(*this) /= ts::get(capacity);
    return *this;
  }
  constexpr Probability<Prec> opposite() { return Probability<Prec>{1 - get(*this)}; }

  explicit constexpr Probability(const Ratio<Prec> &ratio) { get(*this) = get(ratio); }
};

template <typename Prec = mediump>
constexpr auto
operator/(const Probability<Prec> &probability, const Capacity<Prec> &capacity)
{
  return Probability<Prec>{get(probability) / get(capacity)};
}

template <typename Prec = mediump>
constexpr auto operator*(const IntensitySize<Prec> &intensity, const Probability<Prec> &probability)
{
  return Probability<Prec>{get(intensity) * get(probability)};
}

template <typename Prec = mediump>
struct IntensityFactor : ts::strong_typedef<IntensityFactor<Prec>, intensity_t<Prec>>,
                         ts::strong_typedef_op::multiplication<IntensityFactor<Prec>>,
                         ts::strong_typedef_op::output_operator<IntensityFactor<Prec>> {
  using ts::strong_typedef<IntensityFactor<Prec>, intensity_t<Prec>>::strong_typedef;

  constexpr auto operator*(const Intensity<Prec> &intensity) const
  {
    return Intensity<Prec>(ts::get(*this) * ts::get(intensity));
  }
};

template <typename Prec = mediump>
constexpr auto
operator/(const Size<Prec> &size, const IntensityFactor<Prec> &intensity_factor)
{
  return Size<Prec>{static_cast<count_t<Prec>>(get(size) / get(intensity_factor))};
}

template <typename Prec = mediump>
struct Count : ts::strong_typedef<Count<Prec>, count_t<Prec>>,
               ts::strong_typedef_op::equality_comparison<Count<Prec>>,
               ts::strong_typedef_op::increment<Count<Prec>>,
               ts::strong_typedef_op::decrement<Count<Prec>>,
               ts::strong_typedef_op::subtraction<Count<Prec>>,
               ts::strong_typedef_op::addition<Count<Prec>>,
               ts::strong_typedef_op::relational_comparison<Count<Prec>>,
               ts::strong_typedef_op::output_operator<Count<Prec>> {
  using ts::strong_typedef<Count<Prec>, count_t<Prec>>::strong_typedef;
};

template <typename Prec = mediump>
constexpr auto operator*(const Count<Prec> &count, const Intensity<Prec> &intensity)
{
  return ts::get(count) * ts::get(intensity);
}
template <typename Prec = mediump>
constexpr auto
operator/(const Intensity<Prec> &intensity, const Count<Prec> &count)
{
  return Intensity<Prec>{ts::get(intensity) / ts::get(count)};
}
template <typename Prec = mediump>
constexpr auto
operator/(const Count<Prec> &c1, const Count<Prec> &c2)
{
  return Ratio<Prec>{static_cast<ratio_t<Prec>>(ts::get(c1)) / static_cast<ratio_t<Prec>>(ts::get(c2))};
}

template <typename Prec = mediump>
struct Duration : ts::strong_typedef<Duration<Prec>, duration_t<Prec>>,
                  ts::strong_typedef_op::equality_comparison<Duration<Prec>>,
                  ts::strong_typedef_op::subtraction<Duration<Prec>>,
                  ts::strong_typedef_op::addition<Duration<Prec>>,
                  ts::strong_typedef_op::output_operator<Duration<Prec>> {
  using ts::strong_typedef<Duration<Prec>, duration_t<Prec>>::strong_typedef;
  constexpr auto operator/(const Duration<Prec> &duration) const
  {
    return ts::get(*this) / ts::get(duration);
  }

  constexpr auto operator*(const Size<Prec> &size) const
  {
    return Duration<Prec>{ts::get(*this) / ts::get(size)};
  }
};
template <typename Prec = mediump>
struct Variance : ts::strong_typedef<Variance<Prec>, stat_t<Prec>>,
                  ts::strong_typedef_op::addition<Variance<Prec>>,
                  ts::strong_typedef_op::output_operator<Variance<Prec>> {
  using ts::strong_typedef<Variance<Prec>, stat_t<Prec>>::strong_typedef;
  explicit Variance(const Intensity<Prec> &intensity) : Variance(get(intensity)) {}
};
template <typename Prec = mediump>
struct MeanRequestNumber : ts::strong_typedef<MeanRequestNumber<Prec>, intensity_t<Prec>>,
                           ts::strong_typedef_op::addition<MeanRequestNumber<Prec>>,
                           ts::strong_typedef_op::output_operator<MeanRequestNumber<Prec>> {
  using ts::strong_typedef<MeanRequestNumber<Prec>, intensity_t<Prec>>::strong_typedef;
};
template <typename Prec = mediump>
struct MeanIntensity : ts::strong_typedef<MeanIntensity<Prec>, intensity_t<Prec>>,
                       ts::strong_typedef_op::addition<MeanIntensity<Prec>>,
                       ts::strong_typedef_op::output_operator<MeanIntensity<Prec>> {
  using ts::strong_typedef<MeanIntensity<Prec>, intensity_t<Prec>>::strong_typedef;
  explicit operator MeanRequestNumber<Prec>() { return MeanRequestNumber<Prec>{get(*this)}; }
  explicit MeanIntensity(const Intensity<Prec> &intensity) : MeanIntensity(get(intensity)) {}
};

template <typename Prec = mediump>
struct Peakedness : ts::strong_typedef<Peakedness<Prec>, stat_t<Prec>>,
                    ts::strong_typedef_op::addition<Peakedness<Prec>>,
                    ts::strong_typedef_op::relational_comparison<Peakedness<Prec>>,
                    ts::strong_typedef_op::output_operator<Peakedness<Prec>> {
  using ts::strong_typedef<Peakedness<Prec>, stat_t<Prec>>::strong_typedef;
};

template <typename Prec = mediump>
constexpr auto operator*(const Variance<Prec> &variance, const WeightF<Prec> &weight)
{
  return Peakedness<Prec>{get(variance) * get(weight)};
}
template <typename Prec = mediump>
constexpr auto
operator/(const Variance<Prec> &variance, const MeanIntensity<Prec> &mean)
{
  return Peakedness<Prec>{get(variance) / get(mean)};
}
template <typename Prec = mediump>
constexpr auto
operator/(const MeanIntensity<Prec> &mean, const Peakedness<Prec> &peakedness)
{
  return Intensity<Prec>{get(mean) / get(peakedness)};
}

template <typename Prec = mediump>
constexpr auto operator*(const Intensity<Prec> &intensity, const Probability<Prec> &probability)
{
  return MeanIntensity<Prec>{get(intensity) * get(probability)};
}

template <typename Prec = mediump>
constexpr auto
operator/(const Capacity<Prec> &capacity, const Peakedness<Prec> &peakedness)
{
  return CapacityF<Prec>{static_cast<stat_t<Prec>>(get(capacity)) / get(peakedness)};
}
template <typename Prec = mediump>
constexpr auto operator*(const MeanIntensity<Prec> &mean, const Size<Prec> &size)
{
  return WeightF<Prec>{get(mean) * static_cast<weight_float_t<Prec>>(get(size))};
}
template <typename Prec = mediump>
constexpr auto operator*(const MeanRequestNumber<Prec> &mean, const Size<Prec> &size)
{
  return CapacityF<Prec>{get(mean) * static_cast<intensity_t<Prec>>(get(size))};
}
template <typename Prec = mediump>
constexpr auto operator*(const MeanRequestNumber<Prec> &mean, const SizeF<Prec> &size)
{
  return CapacityF<Prec>{get(mean) * static_cast<intensity_t<Prec>>(get(size))};
}
template <typename Prec = mediump>
constexpr auto operator*(const Size<Prec> &size, const InvWeightF<Prec> &inv_weight)
{
  return WeightF<Prec>{static_cast<stat_t<Prec>>(get(size)) * get(inv_weight)};
}

template <typename Prec = mediump>
struct SizeRescale : ts::strong_typedef<SizeRescale<Prec>, intensity_t<Prec>>,
                     ts::strong_typedef_op::output_operator<SizeRescale<Prec>> {
  using ts::strong_typedef<SizeRescale<Prec>, intensity_t<Prec>>::strong_typedef;
  explicit SizeRescale(const Peakedness<Prec> &peakedness) : SizeRescale(get(peakedness)) {}
};

template <typename Prec = mediump>
constexpr auto operator*(const SizeRescale<Prec> &rescale, const Size<Prec> &size)
{
  return SizeF<Prec>{get(rescale) * get(size)};
}
template <typename Prec = mediump>
constexpr auto operator*(const SizeRescale<Prec> &rescale, const SizeF<Prec> &size)
{
  return SizeF<Prec>{get(rescale) * get(size)};
}

template <typename Prec = mediump>
struct Time : ts::strong_typedef<Time<Prec>, time_type<Prec>>,
              ts::strong_typedef_op::equality_comparison<Time<Prec>>,
              ts::strong_typedef_op::relational_comparison<Time<Prec>>,
              ts::strong_typedef_op::output_operator<Time<Prec>> {
  using ts::strong_typedef<Time<Prec>, time_type<Prec>>::strong_typedef;
  explicit constexpr operator Duration<Prec>() const { return Duration<Prec>(ts::get(*this)); }
  constexpr Time<Prec> &operator+=(const Duration<Prec> &d)
  {
    ts::get(*this) += ts::get(d);
    return *this;
  }
  constexpr Time<Prec> operator+(const Duration<Prec> &duration) const
  {
    return Time<Prec>{ts::get(*this) + ts::get(duration)};
  }
  constexpr Duration<Prec> operator-(const Time<Prec> &t) const
  {
    return Duration<Prec>{ts::get(*this) - ts::get(t)};
  }
};
} // namespace Types

using Intensity = Types::Intensity<mediump>;
using IntensityFactor = Types::IntensityFactor<mediump>;
using Ratio = Types::Ratio<mediump>;
using Capacity = Types::Capacity<mediump>;
using Weight = Types::Weight<mediump>;
using WeightF = Types::WeightF<mediump>;
using Size = Types::Size<mediump>;
using SizeF = Types::SizeF<mediump>;
using SizeRescale = Types::SizeRescale<mediump>;
using Count = Types::Count<mediump>;
using Time = Types::Time<mediump>;
using Duration = Types::Duration<mediump>;
using Probability = Types::Probability<mediump>;
using MeanIntensity = Types::MeanIntensity<mediump>;
using Peakedness = Types::Peakedness<mediump>;
using Variance = Types::Variance<mediump>;
using MeanRequestNumber = Types::MeanRequestNumber<mediump>;
using CapacityF = Types::CapacityF<mediump>;


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
