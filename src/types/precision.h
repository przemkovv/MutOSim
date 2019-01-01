
#pragma once

#include "common.h"

#include <type_safe/strong_typedef.hpp>

namespace ts = type_safe;

namespace TypesPrecision
{
template <typename T, typename... Ts>
constexpr bool is_any_same_v = std::disjunction_v<std::is_same<T, Ts>...>;

template <typename... Ts>
using promote_t =
    std::conditional_t<is_any_same_v<use_float_tag, Ts...>, use_float_tag, use_int_tag>;

//----------------------------------------------------------------------
template <typename Prec = mediump>
struct Ratio : ts::strong_typedef<Ratio<Prec>, ratio_t<Prec>>,
               ts::strong_typedef_op::equality_comparison<Ratio<Prec>>,
               ts::strong_typedef_op::multiplication<Ratio<Prec>>,
               ts::strong_typedef_op::output_operator<Ratio<Prec>> {
  using ts::strong_typedef<Ratio<Prec>, ratio_t<Prec>>::strong_typedef;
};

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct InvWeight : ts::strong_typedef<InvWeight<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
                   ts::strong_typedef_op::equality_comparison<InvWeight<Prec, UseFloat>>,
                   ts::strong_typedef_op::addition<InvWeight<Prec, UseFloat>>,
                   ts::strong_typedef_op::output_operator<InvWeight<Prec, UseFloat>> {
  using ts::strong_typedef<InvWeight<Prec, UseFloat>, weight_t<Prec, UseFloat>>::strong_typedef;
};

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Weight : ts::strong_typedef<Weight<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
                ts::strong_typedef_op::equality_comparison<Weight<Prec, UseFloat>>,
                ts::strong_typedef_op::addition<Weight<Prec, UseFloat>>,
                ts::strong_typedef_op::output_operator<Weight<Prec, UseFloat>> {
  using ts::strong_typedef<Weight<Prec, UseFloat>, weight_t<Prec, UseFloat>>::strong_typedef;
  constexpr auto operator/(const Weight<Prec, UseFloat> &w) const
  {
    return Ratio<Prec>{static_cast<ts::underlying_type<Ratio<Prec>>>(ts::get(*this)) / ts::get(w)};
  }
  constexpr auto invert()
  {
    return InvWeight<Prec, use_float_tag>{weight_t<Prec, use_float_tag>{1} / get(*this)};
  }
};

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Capacity : ts::strong_typedef<Capacity<Prec, UseFloat>, count_t<Prec, UseFloat>>,
                  ts::strong_typedef_op::equality_comparison<Capacity<Prec, UseFloat>>,
                  ts::strong_typedef_op::relational_comparison<Capacity<Prec, UseFloat>>,
                  ts::strong_typedef_op::addition<Capacity<Prec, UseFloat>>,
                  ts::strong_typedef_op::subtraction<Capacity<Prec, UseFloat>>,
                  ts::strong_typedef_op::increment<Capacity<Prec, UseFloat>>,
                  ts::strong_typedef_op::output_operator<Capacity<Prec, UseFloat>> {
  using ts::strong_typedef<Capacity<Prec, UseFloat>, count_t<Prec, UseFloat>>::strong_typedef;
  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }

  template <
      typename Dummy = void,
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>, Dummy>>
  constexpr Capacity(const Capacity<Prec, use_int_tag> &c) : Capacity(get(c))
  {
  }

  explicit operator Capacity<Prec, use_int_tag>()
  {
    return Capacity<Prec, use_int_tag>{static_cast<count_t<Prec>>(get(*this))};
  }
};

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Size : ts::strong_typedef<Size<Prec, UseFloat>, count_t<Prec, UseFloat>>,
              ts::strong_typedef_op::equality_comparison<Size<Prec, UseFloat>>,
              ts::strong_typedef_op::relational_comparison<Size<Prec, UseFloat>>,
              ts::strong_typedef_op::addition<Size<Prec, UseFloat>>,
              ts::strong_typedef_op::subtraction<Size<Prec, UseFloat>>,
              ts::strong_typedef_op::output_operator<Size<Prec, UseFloat>> {
  using ts::strong_typedef<Size<Prec, UseFloat>, count_t<Prec, UseFloat>>::strong_typedef;

  template <
      typename Dummy = void,
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>, Dummy>>
  constexpr Size(const Size<Prec, use_int_tag> &c) : Size(get(c))
  {
  }

  constexpr bool operator==(const Capacity<Prec> &c) { return ts::get(*this) == ts::get(c); }
  constexpr bool operator<=(const Capacity<Prec> &c) { return ts::get(*this) <= ts::get(c); }
  constexpr bool operator>(const Capacity<Prec> &c) { return ts::get(*this) > ts::get(c); }
  explicit operator Size<Prec, use_float_tag>()
  {
    return Size<Prec, use_float_tag>{
        static_cast<count_t<Prec, use_float_tag>>(std::ceil(get(*this)))};
  }
};

//----------------------------------------------------------------------
template <
    typename Prec,
    typename CapacityUseFloat,
    typename SizeUseFloat,
    typename CommonUseFloat = promote_t<CapacityUseFloat, SizeUseFloat>>
constexpr auto
operator-(const Capacity<Prec, CapacityUseFloat> &c, const Size<Prec, SizeUseFloat> &s)
{
  return Capacity<Prec, CommonUseFloat>{get(c) - get(s)};
}

template <
    typename Prec,
    typename CapacityUseFloat,
    typename SizeUseFloat,
    typename CommonUseFloat = promote_t<CapacityUseFloat, SizeUseFloat>>
constexpr auto
operator+(const Capacity<Prec, CapacityUseFloat> &c, const Size<Prec, SizeUseFloat> &s)
{
  return Capacity<Prec, CommonUseFloat>{get(c) + get(s)};
}

template <typename Prec, typename UseFloat>
constexpr auto &
operator+=(Capacity<Prec, UseFloat> &c, const Size<Prec, UseFloat> &s)
{
  get(c) = get(c) + get(s);
  return c;
}

template <typename Prec = mediump>
constexpr auto
operator>=(const Capacity<Prec, use_int_tag> &c, const Size<Prec, use_int_tag> &s)
{
  return get(c) >= get(s);
}
//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct IntensitySize
  : ts::strong_typedef<IntensitySize<Prec, UseFloat>, intensity_t<Prec, UseFloat>>,
    ts::strong_typedef_op::multiplication<IntensitySize<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<IntensitySize<Prec, UseFloat>> {
  using ts::strong_typedef<IntensitySize<Prec, UseFloat>, intensity_t<Prec, UseFloat>>::
      strong_typedef;
};

//----------------------------------------------------------------------

template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct Intensity : ts::strong_typedef<Intensity<Prec, UseFloat>, intensity_t<Prec, UseFloat>>,
                   ts::strong_typedef_op::multiplication<Intensity<Prec, UseFloat>>,
                   ts::strong_typedef_op::addition<Intensity<Prec, UseFloat>>,
                   ts::strong_typedef_op::relational_comparison<Intensity<Prec, UseFloat>>,
                   ts::strong_typedef_op::output_operator<Intensity<Prec, UseFloat>> {
  using ts::strong_typedef<Intensity<Prec, UseFloat>, intensity_t<Prec, UseFloat>>::strong_typedef;
  constexpr auto operator/(const Intensity<Prec, UseFloat> &intensity) const
  {
    return Intensity<Prec, UseFloat>{ts::get(*this) / ts::get(intensity)};
  }
  constexpr auto operator/(const Size<Prec> &size) const
  {
    return Intensity<Prec, UseFloat>(ts::get(*this) / ts::get(size));
  }
  template <typename SizeUseFloat>
  constexpr auto operator*(const Size<Prec, SizeUseFloat> &size) const
  {
    return IntensitySize<Prec, promote_t<SizeUseFloat, UseFloat>>(ts::get(*this) * ts::get(size));
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
//----------------------------------------------------------------------

template <typename Prec = mediump>
constexpr bool
operator==(const Intensity<Prec> &intensity1, const Intensity<Prec> &intensity2)
{
  return (get(intensity1) + epsilon) > get(intensity2) &&
         (get(intensity1) - epsilon) < get(intensity2);
}

template <typename Prec = mediump, typename UseFloat = use_float_tag>
constexpr auto
operator/(const IntensitySize<Prec, UseFloat> &intensity, const Size<Prec, UseFloat> &size)
{
  return Intensity<Prec, UseFloat>(ts::get(intensity) / ts::get(size));
}
template <
    typename Prec = mediump,
    typename IntensityUseFloat,
    typename CapacityUseFloat,
    typename UseFloat = promote_t<IntensityUseFloat, CapacityUseFloat>>
constexpr auto
operator/(
    const IntensitySize<Prec, IntensityUseFloat> &intensity,
    const Capacity<Prec, CapacityUseFloat> &size)
{
  return Intensity<Prec, UseFloat>(ts::get(intensity) / ts::get(size));
}
//----------------------------------------------------------------------
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

template <typename Prec = mediump, typename UseFloat = use_float_tag>
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

//----------------------------------------------------------------------
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
//----------------------------------------------------------------------

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
  return Ratio<Prec>{static_cast<ratio_t<Prec>>(ts::get(c1)) /
                     static_cast<ratio_t<Prec>>(ts::get(c2))};
}

//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
template <typename Prec = mediump>
struct Variance : ts::strong_typedef<Variance<Prec>, stat_t<Prec>>,
                  ts::strong_typedef_op::addition<Variance<Prec>>,
                  ts::strong_typedef_op::output_operator<Variance<Prec>> {
  using ts::strong_typedef<Variance<Prec>, stat_t<Prec>>::strong_typedef;
  explicit Variance(const Intensity<Prec> &intensity) : Variance(get(intensity)) {}
};

//----------------------------------------------------------------------

template <typename Prec = mediump>
struct MeanRequestNumber : ts::strong_typedef<MeanRequestNumber<Prec>, intensity_t<Prec>>,
                           ts::strong_typedef_op::addition<MeanRequestNumber<Prec>>,
                           ts::strong_typedef_op::output_operator<MeanRequestNumber<Prec>> {
  using ts::strong_typedef<MeanRequestNumber<Prec>, intensity_t<Prec>>::strong_typedef;
};

//----------------------------------------------------------------------

template <typename Prec = mediump>
struct MeanIntensity : ts::strong_typedef<MeanIntensity<Prec>, intensity_t<Prec>>,
                       ts::strong_typedef_op::addition<MeanIntensity<Prec>>,
                       ts::strong_typedef_op::output_operator<MeanIntensity<Prec>> {
  using ts::strong_typedef<MeanIntensity<Prec>, intensity_t<Prec>>::strong_typedef;
  explicit operator MeanRequestNumber<Prec>() { return MeanRequestNumber<Prec>{get(*this)}; }
  explicit MeanIntensity(const Intensity<Prec> &intensity) : MeanIntensity(get(intensity)) {}
};
//----------------------------------------------------------------------

template <typename Prec = mediump>
struct Peakedness : ts::strong_typedef<Peakedness<Prec>, stat_t<Prec>>,
                    ts::strong_typedef_op::addition<Peakedness<Prec>>,
                    ts::strong_typedef_op::relational_comparison<Peakedness<Prec>>,
                    ts::strong_typedef_op::output_operator<Peakedness<Prec>> {
  using ts::strong_typedef<Peakedness<Prec>, stat_t<Prec>>::strong_typedef;
};

template <typename Prec = mediump>
constexpr auto operator*(const Variance<Prec> &variance, const Weight<Prec, use_float_tag> &weight)
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
operator/(const Capacity<Prec, use_int_tag> &capacity, const Peakedness<Prec> &peakedness)
{
  return Capacity<Prec, use_float_tag>{static_cast<stat_t<Prec>>(get(capacity)) / get(peakedness)};
}
template <typename Prec = mediump>
constexpr auto operator*(const MeanIntensity<Prec> &mean, const Size<Prec> &size)
{
  return Weight<Prec, use_float_tag>{get(mean) *
                                     static_cast<weight_t<Prec, use_float_tag>>(get(size))};
}
template <typename Prec = mediump>
constexpr auto operator*(const MeanRequestNumber<Prec> &mean, const Size<Prec> &size)
{
  return Capacity<Prec, use_float_tag>{get(mean) * static_cast<intensity_t<Prec>>(get(size))};
}
template <typename Prec = mediump>
constexpr auto operator*(const MeanRequestNumber<Prec> &mean, const Size<Prec, use_float_tag> &size)
{
  return Capacity<Prec, use_float_tag>{get(mean) * static_cast<intensity_t<Prec>>(get(size))};
}
template <typename Prec = mediump>
constexpr auto operator*(const Size<Prec> &size, const InvWeight<Prec, use_float_tag> &inv_weight)
{
  return Weight<Prec, use_float_tag>{static_cast<stat_t<Prec>>(get(size)) * get(inv_weight)};
}
//----------------------------------------------------------------------

template <typename Prec = mediump>
struct SizeRescale : ts::strong_typedef<SizeRescale<Prec>, intensity_t<Prec>>,
                     ts::strong_typedef_op::output_operator<SizeRescale<Prec>> {
  using ts::strong_typedef<SizeRescale<Prec>, intensity_t<Prec>>::strong_typedef;
  explicit SizeRescale(const Peakedness<Prec> &peakedness) : SizeRescale(get(peakedness)) {}
};
//----------------------------------------------------------------------

template <typename Prec = mediump>
constexpr auto operator*(const SizeRescale<Prec> &rescale, const Size<Prec> &size)
{
  return Size<Prec, use_float_tag>{get(rescale) * get(size)};
}
template <typename Prec = mediump>
constexpr auto operator*(const SizeRescale<Prec> &rescale, const Size<Prec, use_float_tag> &size)
{
  return Size<Prec, use_float_tag>{get(rescale) * get(size)};
}
//----------------------------------------------------------------------

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
} // namespace TypesPrecision
