
#pragma once

#include "common.h"
#include "logger.h"
#include "operations.h"

#include <type_safe/strong_typedef.hpp>

namespace ts = type_safe;

namespace TypesPrecision {

//----------------------------------------------------------------------
// Template forward declarations

template <typename Prec>
struct Ratio_;

template <typename Prec, typename UseFloat>
struct InvWeight_;

template <typename Prec, typename UseFloat>
struct Weight_;

template <typename Prec>
struct Count_;

template <typename Prec, typename UseFloat>
struct Capacity_;

template <typename Prec, typename UseFloat>
struct Size_;

template <typename Prec>
struct IntensitySize_;

template <typename Prec>
struct Intensity_;

template <typename Prec>
struct Probability_;

template <typename Prec>
struct IntensityFactor_;

template <typename Prec>
struct Duration_;

template <typename Prec>
struct Variance_;

template <typename Prec>
struct MeanRequestNumber_;

template <typename Prec>
struct MeanIntensity_;

template <typename Prec>
struct Peakedness_;

template <typename Prec>
struct SizeRescale_;

template <typename Prec>
struct Time_;

//----------------------------------------------------------------------
template <typename Prec>
struct Ratio_ : ts::strong_typedef<Ratio_<Prec>, ratio_t<Prec>>,
                ts::strong_typedef_op::equality_comparison<Ratio_<Prec>>,
                ts::strong_typedef_op::multiplication<Ratio_<Prec>>,
                ts::strong_typedef_op::output_operator<Ratio_<Prec>>
{
  using ts::strong_typedef<Ratio_<Prec>, ratio_t<Prec>>::strong_typedef;

  using value_type = typename ts::underlying_type<Ratio_>;
};

//----------------------------------------------------------------------
template <typename Prec, typename UseFloat>
struct InvWeight_
  : ts::strong_typedef<InvWeight_<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<InvWeight_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<InvWeight_<Prec, UseFloat>>
{
  using ts::strong_typedef<
      InvWeight_<Prec, UseFloat>,
      weight_t<Prec, UseFloat>>::strong_typedef;

  constexpr auto operator+(const InvWeight_ &w) const
  {
    weight_t<Prec, UseFloat> value = get(*this) + get(w);
    return InvWeight_{value};
  }
};

//----------------------------------------------------------------------
template <typename Prec, typename UseFloat>
struct Weight_
  : ts::strong_typedef<Weight_<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Weight_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Weight_<Prec, UseFloat>>
{
  using ts::strong_typedef<Weight_<Prec, UseFloat>, weight_t<Prec, UseFloat>>::
      strong_typedef;
  using value_type = ts::underlying_type<Weight_>;

  constexpr auto operator/(const Weight_ &w) const
  {
    ratio_t<Prec> result = static_cast<ratio_t<Prec>>(get(*this)) / get(w);
    return Ratio_<Prec>{result};
  }
  constexpr auto invert() const
  {
    weight_t<Prec, use_float_tag> value = 1 / get(*this);
    return InvWeight_<Prec, use_float_tag>{value};
  }
  template <typename Result = Weight_>
  constexpr auto opposite() const
      -> std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>, Result>
  {
    ASSERT(
        get(*this) >= 0 && get(*this) <= 1,
        "[{}] Try to get opposite value while weight is not in normalized "
        "range (0..1)",
        location());
    weight_t<Prec, UseFloat> value = value_type{1} - get(*this);
    return Weight_{value};
  }

  constexpr auto &operator+=(const Weight_ &w)
  {
    ts::get(*this) += ts::get(w);
    return *this;
  }

  constexpr auto operator+(const Weight_ &w2) const
  {
    weight_t<Prec, UseFloat> value = get(*this) + get(w2);
    return Weight_{value};
  }

  constexpr auto operator*(const Probability_<Prec> &probability)
  {
    probability_t<Prec> result = get(*this) * get(probability);
    return Probability_<Prec>{result};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct Count_ : ts::strong_typedef<Count_<Prec>, count_t<Prec>>,
                ts::strong_typedef_op::equality_comparison<Count_<Prec>>,
                ts::strong_typedef_op::increment<Count_<Prec>>,
                ts::strong_typedef_op::decrement<Count_<Prec>>,
                ts::strong_typedef_op::relational_comparison<Count_<Prec>>,
                ts::strong_typedef_op::output_operator<Count_<Prec>>
{
  using ts::strong_typedef<Count_<Prec>, count_t<Prec>>::strong_typedef;

  using value_type = ts::underlying_type<Count_>;
  const auto &value() const { return get(*this); }

  Count_ &operator-=(const Count_ &c)
  {
    get(*this) -= get(c);
    return *this;
  }

  Count_ operator-(const Count_ &c) const
  {
    count_t<Prec> value = get(*this) - get(c);
    return Count_{value};
  }

  Count_ &operator+=(const Count_ &c)
  {
    get(*this) += get(c);
    return *this;
  }

  Count_ operator+(const Count_ &c) const
  {
    count_t<Prec> value = get(*this) + get(c);
    return Count_{value};
  }

  Count_ &operator*=(const Count_ &c)
  {
    get(*this) *= get(c);
    return *this;
  }

  Count_ operator*(const Count_ &c) const
  {
    count_t<Prec> value = get(*this) * get(c);
    return Count_{value};
  }
  constexpr auto operator/(const Count_ &c2) const
  {
    ratio_t<Prec> result = static_cast<ratio_t<Prec>>(get(*this))
                           / static_cast<ratio_t<Prec>>(get(c2));
    return Ratio_<Prec>{result};
  }
  template <typename CapacityUseFloat>
  constexpr auto
  operator*(const Capacity_<Prec, CapacityUseFloat> &capacity) const
  {
    count_t<Prec, CapacityUseFloat> value = mul(get(*this), get(capacity));
    return Capacity_<Prec, CapacityUseFloat>{value};
  }
  constexpr auto operator*(const Intensity_<Prec> &intensity) const
  {
    intensity_t<Prec> value = get(*this) * get(intensity);
    return Intensity_<Prec>{value};
  }
};

//----------------------------------------------------------------------

template <typename Prec, typename UseFloat>
struct Capacity_
  : ts::strong_typedef<Capacity_<Prec, UseFloat>, count_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Capacity_<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Capacity_<Prec, UseFloat>>,
    ts::strong_typedef_op::increment<Capacity_<Prec, UseFloat>>,
    ts::strong_typedef_op::decrement<Capacity_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Capacity_<Prec, UseFloat>>
{
  using ts::strong_typedef<Capacity_<Prec, UseFloat>, count_t<Prec, UseFloat>>::
      strong_typedef;

  using value_type = ts::underlying_type<Capacity_>;
  const auto &value() const { return get(*this); }

  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }

  template <
      typename SrcPrec,
      typename SrcUseFloat,
      typename = std::enable_if_t<!std::is_same_v<
          Capacity_<SrcPrec, SrcUseFloat>,
          Capacity_<Prec, UseFloat>>>>
  explicit constexpr Capacity_(const Capacity_<SrcPrec, SrcUseFloat> &c)
    : Capacity_(static_cast<value_type>(get(c)))
  {
  }
  template <
      typename SrcPrec,
      typename SrcUseFloat,
      typename = std::enable_if_t<std::conjunction_v<
          std::is_same_v<SrcPrec, Prec>,
          std::is_same_v<SrcUseFloat, UseFloat>>>>
  explicit constexpr Capacity_(const Size_<SrcPrec, SrcUseFloat> &s)
    : Capacity_(get(s))
  {
  }
  template <typename DstPrec, typename DstUseFloat>
  explicit operator Capacity_<DstPrec, DstUseFloat>() const &&
  {
    return Capacity_<DstPrec, DstUseFloat>(get(*this));
  }
  constexpr auto &operator+=(const Capacity_ &c)
  {
    get(*this) += get(c);
    return *this;
  }
  constexpr auto &operator-=(const Capacity_ &c)
  {
    get(*this) -= get(c);
    return *this;
  }
  constexpr auto operator-(const Capacity_ &c) const
  {
    count_t<Prec, UseFloat> value = get(*this) - get(c);
    return Capacity_{value};
  }
  constexpr auto operator+(const Capacity_ &c) const
  {
    count_t<Prec, UseFloat> value = get(*this) + get(c);
    return Capacity_{value};
  }

  constexpr auto operator+(const Size_<Prec, UseFloat> &s) const
  {
    count_t<Prec, UseFloat> value = get(*this) + get(s);
    return Capacity_<Prec, UseFloat>{value};
  }

  constexpr auto operator-(const Size_<Prec, UseFloat> &s) const
  {
    count_t<Prec, UseFloat> value = get(*this) - get(s);
    return Capacity_<Prec, UseFloat>{value};
  }

  constexpr auto &operator+=(const Size_<Prec, UseFloat> &s)
  {
    get(*this) = get(*this) + get(s);
    return *this;
  }

  constexpr auto operator>=(const Size_<Prec, UseFloat> &s) const
  {
    return get(*this) >= get(s);
  }
  constexpr auto operator/(const Peakedness_<Prec> &peakedness) const
  {
    count_t<Prec, use_float_tag> result =
        static_cast<stat_t<Prec>>(get(*this)) / get(peakedness);
    return Capacity_<Prec, use_float_tag>{result};
  }
};

//----------------------------------------------------------------------

template <typename Prec, typename UseFloat>
struct Size_
  : ts::strong_typedef<Size_<Prec, UseFloat>, count_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Size_<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Size_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Size_<Prec, UseFloat>>
{
  using ts::strong_typedef<Size_<Prec, UseFloat>, count_t<Prec, UseFloat>>::
      strong_typedef;

  template <typename SrcPrec, typename SrcUseFloat>
  explicit constexpr Size_(const Size_<SrcPrec, SrcUseFloat> &c)
    : Size_(count_t<Prec, UseFloat>(get(c)))
  {
  }
  template <
      typename SrcPrec,
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_int_tag>>>
  explicit constexpr Size_(const Size_<SrcPrec, use_float_tag> &c)
    : Size_(count_t<Prec, UseFloat>(ceil(get(c))))
  {
  }

  template <
      typename Dummy = void,
      typename = std::enable_if_t<std::is_same_v<Prec, mediump>, Dummy>>
  operator Size_<highp, UseFloat>() const
  {
    return Size_<highp, UseFloat>{get(*this)};
  }

  explicit constexpr operator Capacity_<Prec, UseFloat>() const &&
  {
    return Capacity_<Prec, UseFloat>{get(*this)};
  }
  constexpr auto &operator+=(const Size_ &s)
  {
    get(*this) += get(s);
    return *this;
  }
  constexpr auto &operator-=(const Size_ &s)
  {
    get(*this) -= get(s);
    return *this;
  }
  constexpr auto operator-(const Size_ &c2) const
  {
    count_t<Prec, UseFloat> value = get(*this) - get(c2);
    return Size_{value};
  }
  constexpr auto operator+(const Size_ &c2) const
  {
    count_t<Prec, UseFloat> value = get(*this) + get(c2);
    return Size_{value};
  }

  constexpr bool operator==(const Capacity_<Prec, UseFloat> &c) const
  {
    return get(*this) == get(c);
  }
  constexpr bool operator<=(const Capacity_<Prec, UseFloat> &c) const
  {
    return get(*this) <= get(c);
  }
  constexpr bool operator>(const Capacity_<Prec, UseFloat> &c) const
  {
    return get(*this) > get(c);
  }
  constexpr auto operator/(const IntensityFactor_<Prec> &intensity_factor)
  {
    count_t<Prec, use_float_tag> value =
        static_cast<count_t<Prec, use_float_tag>>(
            get(*this) / get(intensity_factor));
    return Size_<Prec, use_float_tag>{value};
  }
  constexpr auto
  operator*(const InvWeight_<Prec, use_float_tag> &inv_weight) const
  {
    using dst_t = weight_t<Prec, use_float_tag>;
    dst_t value = mul(get(*this), get(inv_weight));
    return Weight_<Prec, use_float_tag>{value};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct IntensitySize_
  : ts::strong_typedef<IntensitySize_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::multiplication<IntensitySize_<Prec>>,
    ts::strong_typedef_op::output_operator<IntensitySize_<Prec>>
{
  using ts::strong_typedef<IntensitySize_<Prec>, intensity_t<Prec>>::
      strong_typedef;

  template <typename UseFloat>
  constexpr auto operator/(const Size_<Prec, UseFloat> &size) const
  {
    intensity_t<Prec> value = ts::get(*this) / ts::get(size);
    return Intensity_<Prec>(value);
  }
  template <typename UseFloat>
  constexpr auto operator/(const Capacity_<Prec, UseFloat> &capacity) const
  {
    intensity_t<Prec> value = ts::get(*this) / ts::get(capacity);
    return Intensity_<Prec>(value);
  }

  constexpr auto operator*(const Probability_<Prec> &probability)
  {
    probability_t<Prec> result = get(*this) * get(probability);
    return Probability_<Prec>{result};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct Intensity_
  : ts::strong_typedef<Intensity_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::multiplication<Intensity_<Prec>>,
    ts::strong_typedef_op::addition<Intensity_<Prec>>,
    ts::strong_typedef_op::relational_comparison<Intensity_<Prec>>,
    ts::strong_typedef_op::output_operator<Intensity_<Prec>>
{
  using ts::strong_typedef<Intensity_<Prec>, intensity_t<Prec>>::strong_typedef;

  using value_type = ts::underlying_type<Intensity_>;
  const auto &value() const { return get(*this); }

  constexpr auto operator/(const Intensity_ &intensity) const
  {
    intensity_t<Prec> value = get(*this) / get(intensity);
    return Intensity_<Prec>{value};
  }
  template <typename UseFloat>
  constexpr auto operator/(const Size_<Prec, UseFloat> &size) const
  {
    intensity_t<Prec> value = get(*this) / get(size);
    return Intensity_<Prec>(value);
  }
  constexpr auto operator/(const Count_<Prec> &count) const
  {
    intensity_t<Prec> value = get(*this) / get(count);
    return Intensity_{value};
  }

  template <typename SizeUseFloat>
  constexpr auto operator*(const Size_<Prec, SizeUseFloat> &size) const
  {
    intensity_t<Prec> value = get(*this) * get(size);
    return IntensitySize_<Prec>(value);
  }
  template <typename UseFloat>
  constexpr auto operator*(const Capacity_<Prec, UseFloat> &capacity) const
  {
    intensity_t<Prec> value = get(*this) * get(capacity);
    return Intensity_<Prec>(value);
  }
  constexpr auto operator*(const Ratio_<Prec> &ratio) const
  {
    intensity_t<Prec> value = get(*this) * get(ratio);
    return Intensity_<Prec>(value);
  }

  constexpr bool operator==(const Intensity_<Prec> &intensity) const
  {
    return (get(*this) + epsilon) > get(intensity)
           && (get(*this) - epsilon) < get(intensity);
  }
  constexpr auto operator*(const Probability_<Prec> &probability) const
  {
    intensity_t<Prec> value = get(*this) * get(probability);
    return MeanIntensity_<Prec>{value};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct Probability_
  : ts::strong_typedef<Probability_<Prec>, probability_t<Prec>>,
    ts::strong_typedef_op::relational_comparison<Probability_<Prec>>,
    ts::strong_typedef_op::equality_comparison<Probability_<Prec>>,
    ts::strong_typedef_op::output_operator<Probability_<Prec>>
{
  using ts::strong_typedef<Probability_<Prec>, probability_t<Prec>>::
      strong_typedef;
  using value_type = ts::underlying_type<Probability_>;
  const auto &value() const { return get(*this); }


  constexpr Probability_ opposite()
  {
    probability_t<Prec> value = 1 - get(*this);
    return Probability_{value};
  }

  explicit constexpr Probability_(const Ratio_<Prec> &ratio)
  {
    get(*this) = get(ratio);
  }

  constexpr auto operator+(const Probability_ &p2) const
  {
    probability_t<Prec> value = get(*this) + get(p2);
    return Probability_<Prec>{value};
  }
  constexpr Probability_ &operator+=(const Probability_ &p)
  {
    get(*this) += get(p);
    return *this;
  }

  constexpr auto operator-(const Probability_ &p2) const
  {
    probability_t<Prec> value = get(*this) - get(p2);
    return Probability_<Prec>{value};
  }

  constexpr auto operator*(const Probability_ &p2) const
  {
    probability_t<Prec> value = get(*this) * get(p2);
    return Probability_<Prec>{value};
  }

  constexpr auto operator/(const Probability_ &p2) const
  {
    probability_t<Prec> value = get(*this) / get(p2);
    return Probability_<Prec>{value};
  }
  constexpr Probability_ &operator/=(const Probability_ &p)
  {
    get(*this) /= get(p);
    return *this;
  }

  template <typename UseFloat>
  constexpr auto operator/(const Capacity_<Prec, UseFloat> &capacity) const
  {
    probability_t<Prec> value =
        get(*this) / static_cast<value_type>(get(capacity));
    return Probability_<Prec>{value};
  }
  template <typename UseFloat>
  constexpr Probability_ &operator/=(const Capacity_<Prec, UseFloat> &capacity)
  {
    ts::get(*this) /= static_cast<value_type>(ts::get(capacity));
    return *this;
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct IntensityFactor_
  : ts::strong_typedef<IntensityFactor_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::multiplication<IntensityFactor_<Prec>>,
    ts::strong_typedef_op::output_operator<IntensityFactor_<Prec>>
{
  using ts::strong_typedef<IntensityFactor_<Prec>, intensity_t<Prec>>::
      strong_typedef;

  constexpr auto operator*(const Intensity_<Prec> &intensity) const
  {
    intensity_t<Prec> value = get(*this) * get(intensity);
    return Intensity_<Prec>(value);
  }
};

//----------------------------------------------------------------------

template <typename Prec>
[[deprecated]] constexpr auto
operator*(const Count_<Prec> &count, const Count_<Prec> &count2)
{
  count_t<Prec> result = get(count2) * get(count);
  return Count_<Prec>{result};
}

//----------------------------------------------------------------------

template <typename Prec>
struct Duration_ : ts::strong_typedef<Duration_<Prec>, duration_t<Prec>>,
                   ts::strong_typedef_op::equality_comparison<Duration_<Prec>>,
                   ts::strong_typedef_op::subtraction<Duration_<Prec>>,
                   ts::strong_typedef_op::addition<Duration_<Prec>>,
                   ts::strong_typedef_op::output_operator<Duration_<Prec>>
{
  using ts::strong_typedef<Duration_<Prec>, duration_t<Prec>>::strong_typedef;
  constexpr auto operator/(const Duration_<Prec> &duration) const
  {
    return ts::get(*this) / ts::get(duration);
  }

  constexpr auto operator*(const Size_<Prec, use_int_tag> &size) const
  {
    return Duration_<Prec>{ts::get(*this) / ts::get(size)};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct Variance_ : ts::strong_typedef<Variance_<Prec>, stat_t<Prec>>,
                   ts::strong_typedef_op::addition<Variance_<Prec>>,
                   ts::strong_typedef_op::output_operator<Variance_<Prec>>
{
  using ts::strong_typedef<Variance_<Prec>, stat_t<Prec>>::strong_typedef;
  using value_type = ts::underlying_type<Variance_>;

  template <typename Prec2>
  explicit Variance_(const Intensity_<Prec2> &intensity)
    : Variance_(get(intensity))
  {
  }
  constexpr auto operator*(const Weight_<Prec, use_float_tag> &weight) const
  {
    stat_t<Prec> value = get(*this) * get(weight);
    return Peakedness_<Prec>{value};
  }
  constexpr auto operator/(const MeanIntensity_<Prec> &mean) const
  {
    stat_t<Prec> value = get(*this) / get(mean);
    return Peakedness_<Prec>{value};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct MeanRequestNumber_
  : ts::strong_typedef<MeanRequestNumber_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::addition<MeanRequestNumber_<Prec>>,
    ts::strong_typedef_op::output_operator<MeanRequestNumber_<Prec>>
{
  using ts::strong_typedef<MeanRequestNumber_<Prec>, intensity_t<Prec>>::
      strong_typedef;

  template <typename UseFloat>
  constexpr auto operator*(const Size_<Prec, UseFloat> &size) const
  {
    using dst_t = count_t<Prec, use_float_tag>;
    dst_t result = mul(get(*this), get(size));
    return Capacity_<Prec, use_float_tag>{result};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct MeanIntensity_
  : ts::strong_typedef<MeanIntensity_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::addition<MeanIntensity_<Prec>>,
    ts::strong_typedef_op::output_operator<MeanIntensity_<Prec>>
{
  using ts::strong_typedef<MeanIntensity_<Prec>, intensity_t<Prec>>::
      strong_typedef;

  MeanIntensity_(const Intensity_<Prec> &intensity)
    : MeanIntensity_(get(intensity))
  {
  }

  explicit operator MeanRequestNumber_<Prec>() const
  {
    return MeanRequestNumber_<Prec>{get(*this)};
  }

  constexpr auto operator/(const Peakedness_<Prec> &peakedness) const
  {
    intensity_t<Prec> value = get(*this) / get(peakedness);
    return Intensity_<Prec>{value};
  }

  template <typename UseFloat>
  constexpr auto operator*(const Size_<Prec, UseFloat> &size) const
  {
    using dst_t = weight_t<Prec, use_float_tag>;
    dst_t value = mul(get(*this), get(size));
    return Weight_<Prec, use_float_tag>{value};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct Peakedness_
  : ts::strong_typedef<Peakedness_<Prec>, stat_t<Prec>>,
    ts::strong_typedef_op::relational_comparison<Peakedness_<Prec>>,
    ts::strong_typedef_op::output_operator<Peakedness_<Prec>>
{
  using ts::strong_typedef<Peakedness_<Prec>, stat_t<Prec>>::strong_typedef;

  constexpr auto operator+(const Peakedness_<Prec> &w2) const
  {
    stat_t<Prec> value = get(*this) + get(w2);
    return Peakedness_<Prec>{value};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct SizeRescale_ : ts::strong_typedef<SizeRescale_<Prec>, intensity_t<Prec>>,
                      ts::strong_typedef_op::output_operator<SizeRescale_<Prec>>
{
  using ts::strong_typedef<SizeRescale_<Prec>, intensity_t<Prec>>::
      strong_typedef;

  explicit SizeRescale_(const Peakedness_<Prec> &peakedness)
    : SizeRescale_(get(peakedness))
  {
  }
  template <typename SizeUseFloat>
  constexpr auto operator*(const Size_<Prec, SizeUseFloat> &size) const
  {
    using dst_t = count_t<Prec, use_float_tag>;
    dst_t value = mul(get(*this), get(size));
    return Size_<Prec, use_float_tag>{value};
  }
};

//----------------------------------------------------------------------

template <typename Prec>
struct Time_ : ts::strong_typedef<Time_<Prec>, time_type<Prec>>,
               ts::strong_typedef_op::equality_comparison<Time_<Prec>>,
               ts::strong_typedef_op::relational_comparison<Time_<Prec>>,
               ts::strong_typedef_op::output_operator<Time_<Prec>>
{
  using ts::strong_typedef<Time_<Prec>, time_type<Prec>>::strong_typedef;
  explicit constexpr operator Duration_<Prec>() const
  {
    return Duration_<Prec>(ts::get(*this));
  }
  constexpr Time_ &operator+=(const Duration_<Prec> &d)
  {
    ts::get(*this) += ts::get(d);
    return *this;
  }
  constexpr Time_ operator+(const Duration_<Prec> &duration) const
  {
    return Time_{ts::get(*this) + ts::get(duration)};
  }
  constexpr Duration_<Prec> operator-(const Time_ &t) const
  {
    return Duration_<Prec>{ts::get(*this) - ts::get(t)};
  }
};

//----------------------------------------------------------------------

} // namespace TypesPrecision
