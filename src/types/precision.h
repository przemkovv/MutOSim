
#pragma once

#include "common.h"
#include "logger.h"

#include <type_safe/strong_typedef.hpp>

namespace ts = type_safe;

namespace TypesPrecision {
template <typename T, typename... Ts>
constexpr bool is_any_same_v = std::disjunction_v<std::is_same<T, Ts>...>;

template <typename... Ts>
using promote_t = std::conditional_t<
    is_any_same_v<use_float_tag, Ts...>,
    use_float_tag,
    use_int_tag>;

template <typename... Ts>
using precision_t =
    std::conditional_t<is_any_same_v<highp, Ts...>, highp, mediump>;

//----------------------------------------------------------------------
template <typename Prec = mediump>
struct Ratio_ : ts::strong_typedef<Ratio_<Prec>, ratio_t<Prec>>,
                ts::strong_typedef_op::equality_comparison<Ratio_<Prec>>,
                ts::strong_typedef_op::multiplication<Ratio_<Prec>>,
                ts::strong_typedef_op::output_operator<Ratio_<Prec>>
{
  using ts::strong_typedef<Ratio_<Prec>, ratio_t<Prec>>::strong_typedef;
};

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct InvWeight_
  : ts::strong_typedef<InvWeight_<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<InvWeight_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<InvWeight_<Prec, UseFloat>>
{
  using ts::strong_typedef<
      InvWeight_<Prec, UseFloat>,
      weight_t<Prec, UseFloat>>::strong_typedef;
};

template <typename Prec, typename UseFloat>
constexpr auto
operator+(
    const InvWeight_<Prec, UseFloat> &w1,
    const InvWeight_<Prec, UseFloat> &w2)
{
  weight_t<Prec, UseFloat> value = get(w1) + get(w2);
  return InvWeight_<Prec, UseFloat>{value};
}

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Weight_
  : ts::strong_typedef<Weight_<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Weight_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Weight_<Prec, UseFloat>>
{
  using ts::strong_typedef<Weight_<Prec, UseFloat>, weight_t<Prec, UseFloat>>::
      strong_typedef;
  constexpr auto operator/(const Weight_ &w) const
  {
    return Ratio_<Prec>{
        static_cast<ts::underlying_type<Ratio_<Prec>>>(ts::get(*this))
        / ts::get(w)};
  }
  constexpr auto invert()
  {
    weight_t<Prec, use_float_tag> value = 1 / get(*this);
    return InvWeight_<Prec, use_float_tag>{value};
  }
  template <
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>>>
  constexpr Weight_ opposite()
  {
    ASSERT(
        get(*this) >= 0 && get(*this) <= 1,
        "[{}] Try to get opposite value while weight is not in normalized "
        "range (0..1)",
        location());
    weight_t<Prec, UseFloat> value = 1 - get(*this);
    return Weight_{value};
  }

  constexpr auto &operator+=(const Weight_ &w)
  {
    ts::get(*this) += ts::get(w);
    return *this;
  }
};

template <typename Prec, typename UseFloat>
constexpr auto
operator+(const Weight_<Prec, UseFloat> &w1, const Weight_<Prec, UseFloat> &w2)
{
  weight_t<Prec, UseFloat> value = get(w1) + get(w2);
  return Weight_<Prec, UseFloat>{value};
}
//----------------------------------------------------------------------

template <typename Prec = mediump>
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
};
//----------------------------------------------------------------------
//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Capacity_
  : ts::strong_typedef<Capacity_<Prec, UseFloat>, count_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Capacity_<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Capacity_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Capacity_<Prec, UseFloat>>
{
  using ts::strong_typedef<Capacity_<Prec, UseFloat>, count_t<Prec, UseFloat>>::
      strong_typedef;

  using value_type = ts::underlying_type<Capacity_>;
  const auto &value() const { return get(*this); }

  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }

  template <
      typename Prec2,
      typename Dummy = void,
      typename =
          std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>, Dummy>>
  constexpr Capacity_(const Capacity_<Prec2, use_int_tag> &c)
    : Capacity_(get(c))
  {
  }

  template <typename Prec2>
  explicit operator Capacity_<Prec2, use_int_tag>() const
  {
    return Capacity_<Prec2, use_int_tag>{
        static_cast<count_t<Prec2>>(get(*this))};
  }
  template <
      typename Dummy = void,
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_int_tag>, Dummy>>
  Capacity_ &operator++()
  {
    ++ts::get(*this);
    return *this;
  }
  template <
      typename Dummy = void,
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_int_tag>, Dummy>>
  Capacity_ operator++(int)
  {
    const Capacity_ result = *this;
    ts::get(*this)++;
    return result;
  }
  constexpr auto &operator+=(const Capacity_ &c)
  {
    ts::get(*this) += ts::get(c);
    return *this;
  }
  constexpr auto &operator-=(const Capacity_ &c)
  {
    ts::get(*this) -= ts::get(c);
    return *this;
  }
};

template <typename Prec, typename UseFloat>
constexpr auto
operator-(
    const Capacity_<Prec, UseFloat> &c1,
    const Capacity_<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) - get(c2);
  return Capacity_<Prec, UseFloat>{value};
}
template <typename Prec, typename UseFloat>
constexpr auto
operator+(
    const Capacity_<Prec, UseFloat> &c1,
    const Capacity_<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) + get(c2);
  return Capacity_<Prec, UseFloat>{value};
}

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Size_
  : ts::strong_typedef<Size_<Prec, UseFloat>, count_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Size_<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Size_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Size_<Prec, UseFloat>>
{
  using ts::strong_typedef<Size_<Prec, UseFloat>, count_t<Prec, UseFloat>>::
      strong_typedef;

  template <typename Prec2, typename UseFloat2>
  constexpr Size_(const Size_<Prec2, UseFloat2> &c)
    : Size_(count_t<Prec, UseFloat>(get(c)))
  {
  }
  constexpr auto &operator+=(const Size_ &s)
  {
    return ts::get(*this) += ts::get(s);
  }
  constexpr auto &operator-=(const Size_ &s)
  {
    return ts::get(*this) -= ts::get(s);
  }

  constexpr bool operator==(const Capacity_<Prec> &c)
  {
    return ts::get(*this) == ts::get(c);
  }
  constexpr bool operator<=(const Capacity_<Prec> &c)
  {
    return ts::get(*this) <= ts::get(c);
  }
  constexpr bool operator>(const Capacity_<Prec> &c)
  {
    return ts::get(*this) > ts::get(c);
  }
  explicit operator Size_<Prec, use_int_tag>() const
  {
    return Size_<Prec, use_int_tag>{
        static_cast<count_t<Prec, use_int_tag>>(std::ceil(get(*this)))};
  }
};

template <typename Prec, typename UseFloat>
constexpr auto
operator-(const Size_<Prec, UseFloat> &c1, const Size_<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) - get(c2);
  return Size_<Prec, UseFloat>{value};
}
template <typename Prec, typename UseFloat>
constexpr auto
operator+(const Size_<Prec, UseFloat> &c1, const Size_<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) + get(c2);
  return Size_<Prec, UseFloat>{value};
}

//----------------------------------------------------------------------
template <
    typename Prec,
    typename Prec2,
    typename CapacityUseFloat,
    typename SizeUseFloat,
    typename CommonUseFloat = promote_t<CapacityUseFloat, SizeUseFloat>,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator-(
    const Capacity_<Prec, CapacityUseFloat> &c,
    const Size_<Prec2, SizeUseFloat> &       s)
{
  count_t<Result, CommonUseFloat> value = get(c) - get(s);
  return Capacity_<Result, CommonUseFloat>{value};
}

template <
    typename Prec,
    typename Prec2,
    typename CapacityUseFloat,
    typename SizeUseFloat,
    typename CommonUseFloat = promote_t<CapacityUseFloat, SizeUseFloat>,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator+(
    const Capacity_<Prec, CapacityUseFloat> &c,
    const Size_<Prec2, SizeUseFloat> &       s)
{
  count_t<Result, CommonUseFloat> value = get(c) + get(s);
  return Capacity_<Result, CommonUseFloat>{value};
}

template <typename Prec, typename UseFloat>
constexpr auto &
operator+=(Capacity_<Prec, UseFloat> &c, const Size_<Prec, UseFloat> &s)
{
  get(c) = get(c) + get(s);
  return c;
}

template <typename Prec = mediump>
constexpr auto
operator>=(
    const Capacity_<Prec, use_int_tag> &c,
    const Size_<Prec, use_int_tag> &    s)
{
  return get(c) >= get(s);
}
//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct IntensitySize_
  : ts::strong_typedef<
        IntensitySize_<Prec, UseFloat>,
        intensity_t<Prec, UseFloat>>,
    ts::strong_typedef_op::multiplication<IntensitySize_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<IntensitySize_<Prec, UseFloat>>
{
  using ts::strong_typedef<
      IntensitySize_<Prec, UseFloat>,
      intensity_t<Prec, UseFloat>>::strong_typedef;
};

//----------------------------------------------------------------------

template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct Intensity_
  : ts::strong_typedef<Intensity_<Prec, UseFloat>, intensity_t<Prec, UseFloat>>,
    ts::strong_typedef_op::multiplication<Intensity_<Prec, UseFloat>>,
    ts::strong_typedef_op::addition<Intensity_<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Intensity_<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Intensity_<Prec, UseFloat>>
{
  using ts::strong_typedef<
      Intensity_<Prec, UseFloat>,
      intensity_t<Prec, UseFloat>>::strong_typedef;
  template <
      typename Prec2,
      typename = std::enable_if<std::is_same_v<precision_t<Prec, Prec2>, Prec>>>
  Intensity_(const Intensity_<Prec2> &intensity) : Intensity_(get(intensity))
  {
  }
  constexpr auto operator/(const Intensity_<Prec, UseFloat> &intensity) const
  {
    return Intensity_<Prec, UseFloat>{ts::get(*this) / ts::get(intensity)};
  }
  constexpr auto operator/(const Size_<Prec> &size) const
  {
    return Intensity_<Prec, UseFloat>(ts::get(*this) / ts::get(size));
  }
  template <
      typename Prec2 = mediump,
      typename SizeUseFloat,
      typename Result = precision_t<Prec, Prec2>>
  constexpr auto operator*(const Size_<Prec2, SizeUseFloat> &size) const
  {
    typename Result::float_t value = get(*this) * get(size);
    return IntensitySize_<Result, promote_t<SizeUseFloat, UseFloat>>(value);
  }
  constexpr auto operator*(const Capacity_<Prec> &capacity) const
  {
    return Intensity_<Prec>(ts::get(*this) * ts::get(capacity));
  }
  constexpr auto operator*(const Ratio_<Prec> &ratio) const
  {
    return Intensity_<Prec>(ts::get(*this) * ts::get(ratio));
  }
};
//----------------------------------------------------------------------

template <typename Prec = mediump>
constexpr bool
operator==(
    const Intensity_<Prec> &intensity1,
    const Intensity_<Prec> &intensity2)
{
  return (get(intensity1) + epsilon) > get(intensity2)
         && (get(intensity1) - epsilon) < get(intensity2);
}

template <typename Prec = mediump, typename UseFloat = use_float_tag>
constexpr auto
operator/(
    const IntensitySize_<Prec, UseFloat> &intensity,
    const Size_<Prec, UseFloat> &         size)
{
  return Intensity_<Prec, UseFloat>(ts::get(intensity) / ts::get(size));
}
template <
    typename Prec = mediump,
    typename IntensityUseFloat,
    typename CapacityUseFloat,
    typename UseFloat = promote_t<IntensityUseFloat, CapacityUseFloat>>
constexpr auto
operator/(
    const IntensitySize_<Prec, IntensityUseFloat> &intensity,
    const Capacity_<Prec, CapacityUseFloat> &      size)
{
  return Intensity_<Prec, UseFloat>(ts::get(intensity) / ts::get(size));
}
//----------------------------------------------------------------------
template <typename Prec = mediump>
struct Probability_
  : ts::strong_typedef<Probability_<Prec>, probability_t<Prec>>,
    ts::strong_typedef_op::relational_comparison<Probability_<Prec>>,
    ts::strong_typedef_op::equality_comparison<Probability_<Prec>>,
    ts::strong_typedef_op::output_operator<Probability_<Prec>>
{
  using ts::strong_typedef<Probability_<Prec>, probability_t<Prec>>::
      strong_typedef;

  template <typename Prec2, typename UseFloat>
  constexpr Probability_ &operator/=(const Capacity_<Prec2, UseFloat> &capacity)
  {
    ts::get(*this) /= ts::get(capacity);
    return *this;
  }
  constexpr Probability_ opposite()
  {
    probability_t<Prec> value = 1 - get(*this);
    return Probability_{value};
  }
  constexpr Probability_ &operator/=(const Probability_ &p)
  {
    get(*this) /= get(p);
    return *this;
  }
  constexpr Probability_ &operator+=(const Probability_ &p)
  {
    get(*this) += get(p);
    return *this;
  }

  template <typename Prec2>
  explicit constexpr Probability_(const Ratio_<Prec2> &ratio)
  {
    get(*this) = get(ratio);
  }
};

template <typename Prec>
constexpr auto
operator+(const Probability_<Prec> &p1, const Probability_<Prec> &p2)
{
  probability_t<Prec> value = get(p1) + get(p2);
  return Probability_<Prec>{value};
}

template <typename Prec>
constexpr auto
operator-(const Probability_<Prec> &p1, const Probability_<Prec> &p2)
{
  probability_t<Prec> value = get(p1) - get(p2);
  return Probability_<Prec>{value};
}

template <typename Prec>
constexpr auto
operator*(const Probability_<Prec> &p1, const Probability_<Prec> &p2)
{
  probability_t<Prec> value = get(p1) * get(p2);
  return Probability_<Prec>{value};
}

template <typename Prec>
constexpr auto
operator/(const Probability_<Prec> &p1, const Probability_<Prec> &p2)
{
  probability_t<Prec> value = get(p1) / get(p2);
  return Probability_<Prec>{value};
}

template <typename Prec = mediump>
constexpr auto
operator/(
    const Probability_<Prec> &probability,
    const Capacity_<Prec> &   capacity)
{
  return Probability_<Prec>{get(probability) / get(capacity)};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename UseFloat,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const Weight_<Prec, UseFloat> &weight,
    const Probability_<Prec2> &    probability)
{
  probability_t<Result> result = get(weight) * get(probability);
  return Probability_<Result>{result};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const IntensitySize_<Prec> &intensity,
    const Probability_<Prec2> & probability)
{
  probability_t<Result> result = get(intensity) * get(probability);
  return Probability_<Result>{result};
}

template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct IntensityFactor_
  : ts::strong_typedef<IntensityFactor_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::multiplication<IntensityFactor_<Prec>>,
    ts::strong_typedef_op::output_operator<IntensityFactor_<Prec>>
{
  using ts::strong_typedef<IntensityFactor_<Prec>, intensity_t<Prec>>::
      strong_typedef;

  constexpr auto operator*(const Intensity_<Prec> &intensity) const
  {
    return Intensity_<Prec>(ts::get(*this) * ts::get(intensity));
  }
};

template <typename Prec = mediump>
constexpr auto
operator/(
    const Size_<Prec> &           size,
    const IntensityFactor_<Prec> &intensity_factor)
{
  return Size_<Prec>{
      static_cast<count_t<Prec>>(get(size) / get(intensity_factor))};
}

//----------------------------------------------------------------------

template <typename Prec = mediump>
constexpr auto
operator*(const Count_<Prec> &count, const Intensity_<Prec> &intensity)
{
  return ts::get(count) * ts::get(intensity);
}

template <typename Prec = mediump>
constexpr auto
operator/(const Intensity_<Prec> &intensity, const Count_<Prec> &count)
{
  return Intensity_<Prec>{ts::get(intensity) / ts::get(count)};
}

template <typename Prec = mediump>
constexpr auto
operator/(const Count_<Prec> &c1, const Count_<Prec> &c2)
{
  ratio_t<Prec> result = static_cast<ratio_t<Prec>>(ts::get(c1))
                         / static_cast<ratio_t<Prec>>(ts::get(c2));
  return Ratio_<Prec>{result};
}

template <typename Prec>
constexpr auto operator*(const Count_<Prec> &count, const Count_<Prec> &count2)
{
  count_t<Prec> result = get(count2) * get(count);
  return Count_<Prec>{result};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename CapacityUseFloat,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator
    *(const Count_<Prec> &count, const Capacity_<Prec2, CapacityUseFloat> &c)
{
  count_t<Result, CapacityUseFloat> value = get(c) * get(count);
  return Capacity_<Result, CapacityUseFloat>{value};
}

//----------------------------------------------------------------------
template <typename Prec = mediump>
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

  constexpr auto operator*(const Size_<Prec> &size) const
  {
    return Duration_<Prec>{ts::get(*this) / ts::get(size)};
  }
};

//----------------------------------------------------------------------
template <typename Prec = mediump>
struct Variance_ : ts::strong_typedef<Variance_<Prec>, stat_t<Prec>>,
                   ts::strong_typedef_op::addition<Variance_<Prec>>,
                   ts::strong_typedef_op::output_operator<Variance_<Prec>>
{
  using ts::strong_typedef<Variance_<Prec>, stat_t<Prec>>::strong_typedef;
  template <typename Prec2>
  explicit Variance_(const Intensity_<Prec2> &intensity)
    : Variance_(get(intensity))
  {
  }
};

//----------------------------------------------------------------------

template <typename Prec = mediump>
struct MeanRequestNumber_
  : ts::strong_typedef<MeanRequestNumber_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::addition<MeanRequestNumber_<Prec>>,
    ts::strong_typedef_op::output_operator<MeanRequestNumber_<Prec>>
{
  using ts::strong_typedef<MeanRequestNumber_<Prec>, intensity_t<Prec>>::
      strong_typedef;
};

//----------------------------------------------------------------------

template <typename Prec = mediump>
struct MeanIntensity_
  : ts::strong_typedef<MeanIntensity_<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::addition<MeanIntensity_<Prec>>,
    ts::strong_typedef_op::output_operator<MeanIntensity_<Prec>>
{
  using ts::strong_typedef<MeanIntensity_<Prec>, intensity_t<Prec>>::
      strong_typedef;
  explicit operator MeanRequestNumber_<Prec>()
  {
    return MeanRequestNumber_<Prec>{get(*this)};
  }
  template <
      typename Prec2,
      typename = std::enable_if<std::is_same_v<precision_t<Prec, Prec2>, Prec>>>
  MeanIntensity_(const Intensity_<Prec2> &intensity)
    : MeanIntensity_(get(intensity))
  {
  }
};
//----------------------------------------------------------------------

template <typename Prec = mediump>
struct Peakedness_
  : ts::strong_typedef<Peakedness_<Prec>, stat_t<Prec>>,
    ts::strong_typedef_op::relational_comparison<Peakedness_<Prec>>,
    ts::strong_typedef_op::output_operator<Peakedness_<Prec>>
{
  using ts::strong_typedef<Peakedness_<Prec>, stat_t<Prec>>::strong_typedef;
};

template <typename Prec>
constexpr auto
operator+(const Peakedness_<Prec> &w1, const Peakedness_<Prec> &w2)
{
  stat_t<Prec> value = get(w1) + get(w2);
  return Peakedness_<Prec>{value};
}

template <typename Prec = mediump>
constexpr auto operator*(
    const Variance_<Prec> &             variance,
    const Weight_<Prec, use_float_tag> &weight)
{
  stat_t<Prec> value = get(variance) * get(weight);
  return Peakedness_<Prec>{value};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator/(const Variance_<Prec> &variance, const MeanIntensity_<Prec2> &mean)
    -> Peakedness_<Result>
{
  typename Result::float_t value = get(variance) / get(mean);
  return Peakedness_<Result>{value};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator/(
    const MeanIntensity_<Prec> &mean,
    const Peakedness_<Prec2> &  peakedness) -> Intensity_<Result>
{
  typename Result::float_t value = get(mean) / get(peakedness);
  return Intensity_<Result>{value};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const Intensity_<Prec> &   intensity,
    const Probability_<Prec2> &probability) -> MeanIntensity_<Result>
{
  typename Result::float_t value = get(intensity) * get(probability);
  return MeanIntensity_<Result>(value);
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename UseFloat,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator/(
    const Capacity_<Prec, UseFloat> &capacity,
    const Peakedness_<Prec2> &peakedness) -> Capacity_<Result, use_float_tag>
{
  count_t<Result, use_float_tag> result =
      static_cast<stat_t<Prec>>(get(capacity)) / get(peakedness);
  return Capacity_<Result, use_float_tag>{result};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator*(const MeanIntensity_<Prec> &mean, const Size_<Prec2> &size)
    -> Weight_<Result, use_float_tag>
{
  typename Result::float_t value = get(mean) * get(size);
  return Weight_<Result, use_float_tag>{value};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator*(const MeanRequestNumber_<Prec> &mean, const Size_<Prec2> &size)
{
  count_t<Result, use_float_tag> result = get(mean) * get(size);
  return Capacity_<Result, use_float_tag>{result};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const MeanRequestNumber_<Prec> &   mean,
    const Size_<Prec2, use_float_tag> &size)
{
  count_t<Result, use_float_tag> value = get(mean) * get(size);
  return Capacity_<Result, use_float_tag>{value};
}
template <
    typename PrecSize = mediump,
    typename PrecWeight = mediump,
    typename Result = precision_t<PrecSize, PrecWeight>>
constexpr auto operator*(
    const Size_<PrecSize> &                      size,
    const InvWeight_<PrecWeight, use_float_tag> &inv_weight)
{
  weight_t<Result, use_float_tag> value = get(size) * get(inv_weight);
  return Weight_<Result, use_float_tag>{value};
}
//----------------------------------------------------------------------

template <typename Prec = mediump>
struct SizeRescale_ : ts::strong_typedef<SizeRescale_<Prec>, intensity_t<Prec>>,
                      ts::strong_typedef_op::output_operator<SizeRescale_<Prec>>
{
  using ts::strong_typedef<SizeRescale_<Prec>, intensity_t<Prec>>::
      strong_typedef;
  explicit SizeRescale_(const Peakedness_<Prec> &peakedness)
    : SizeRescale_(get(peakedness))
  {
  }
};
//----------------------------------------------------------------------

template <
    typename Prec = mediump,
    typename PrecSize = mediump,
    typename SizeUseFloat,
    typename Result = precision_t<Prec, PrecSize>>
constexpr auto operator*(
    const SizeRescale_<Prec> &           rescale,
    const Size_<PrecSize, SizeUseFloat> &size)
{
  count_t<Result, use_float_tag> value = get(rescale) * get(size);
  return Size_<Result, use_float_tag>{value};
}
//----------------------------------------------------------------------

template <typename Prec = mediump>
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
  constexpr Time_<Prec> &operator+=(const Duration_<Prec> &d)
  {
    ts::get(*this) += ts::get(d);
    return *this;
  }
  constexpr Time_<Prec> operator+(const Duration_<Prec> &duration) const
  {
    return Time_<Prec>{ts::get(*this) + ts::get(duration)};
  }
  constexpr Duration_<Prec> operator-(const Time_<Prec> &t) const
  {
    return Duration_<Prec>{ts::get(*this) - ts::get(t)};
  }
};
} // namespace TypesPrecision
