
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
struct Ratio : ts::strong_typedef<Ratio<Prec>, ratio_t<Prec>>,
               ts::strong_typedef_op::equality_comparison<Ratio<Prec>>,
               ts::strong_typedef_op::multiplication<Ratio<Prec>>,
               ts::strong_typedef_op::output_operator<Ratio<Prec>>
{
  using ts::strong_typedef<Ratio<Prec>, ratio_t<Prec>>::strong_typedef;
};

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct InvWeight
  : ts::strong_typedef<InvWeight<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<InvWeight<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<InvWeight<Prec, UseFloat>>
{
  using ts::strong_typedef<
      InvWeight<Prec, UseFloat>,
      weight_t<Prec, UseFloat>>::strong_typedef;
};

template <typename Prec, typename UseFloat>
constexpr auto
operator+(
    const InvWeight<Prec, UseFloat> &w1,
    const InvWeight<Prec, UseFloat> &w2)
{
  weight_t<Prec, UseFloat> value = get(w1) + get(w2);
  return InvWeight<Prec, UseFloat>{value};
}

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Weight
  : ts::strong_typedef<Weight<Prec, UseFloat>, weight_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Weight<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Weight<Prec, UseFloat>>
{
  using ts::strong_typedef<Weight<Prec, UseFloat>, weight_t<Prec, UseFloat>>::
      strong_typedef;
  constexpr auto operator/(const Weight &w) const
  {
    return Ratio<Prec>{
        static_cast<ts::underlying_type<Ratio<Prec>>>(ts::get(*this))
        / ts::get(w)};
  }
  constexpr auto invert()
  {
    weight_t<Prec, use_float_tag> value = 1 / get(*this);
    return InvWeight<Prec, use_float_tag>{value};
  }
  template <
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>>>
  constexpr Weight opposite()
  {
    ASSERT(
        get(*this) >= 0 && get(*this) <= 1,
        "[{}] Try to get opposite value while weight is not in normalized "
        "range (0..1)",
        location());
    weight_t<Prec, UseFloat> value = 1 - get(*this);
    return Weight{value};
  }

  constexpr auto &operator+=(const Weight &w)
  {
    ts::get(*this) += ts::get(w);
    return *this;
  }
};

template <typename Prec, typename UseFloat>
constexpr auto
operator+(const Weight<Prec, UseFloat> &w1, const Weight<Prec, UseFloat> &w2)
{
  weight_t<Prec, UseFloat> value = get(w1) + get(w2);
  return Weight<Prec, UseFloat>{value};
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
               ts::strong_typedef_op::output_operator<Count<Prec>>
{
  using ts::strong_typedef<Count<Prec>, count_t<Prec>>::strong_typedef;

  using value_type = ts::underlying_type<Count>;
  const auto &value() { return get(*this); }

};
//----------------------------------------------------------------------
//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Capacity
  : ts::strong_typedef<Capacity<Prec, UseFloat>, count_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Capacity<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Capacity<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Capacity<Prec, UseFloat>>
{
  using ts::strong_typedef<Capacity<Prec, UseFloat>, count_t<Prec, UseFloat>>::
      strong_typedef;

  using value_type = ts::underlying_type<Capacity>;
  using difference_type = typename Capacity::value_type;
  const auto &value() { return get(*this); }

  // Capacity(const count_t<Prec> &value) : Capacity(value) {}

  explicit operator size_t() const { return static_cast<size_t>(get(*this)); }

  template <
      typename Prec2,
      typename Dummy = void,
      typename =
          std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>, Dummy>>
  constexpr Capacity(const Capacity<Prec2, use_int_tag> &c) : Capacity(get(c))
  {
  }

  template <typename Prec2>
  explicit operator Capacity<Prec2, use_int_tag>() const
  {
    return Capacity<Prec2, use_int_tag>{
        static_cast<count_t<Prec2>>(get(*this))};
  }
  template <
      typename Dummy = void,
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_int_tag>, Dummy>>
  Capacity &operator++()
  {
    ++ts::get(*this);
    return *this;
  }
  template <
      typename Dummy = void,
      typename = std::enable_if_t<std::is_same_v<UseFloat, use_int_tag>, Dummy>>
  Capacity operator++(int)
  {
    const Capacity result = *this;
    ts::get(*this)++;
    return result;
  }
  constexpr auto &operator+=(const Capacity &c)
  {
    ts::get(*this) += ts::get(c);
    return *this;
  }
  constexpr auto &operator-=(const Capacity &c)
  {
    ts::get(*this) -= ts::get(c);
    return *this;
  }
};

template <typename Prec, typename UseFloat>
constexpr auto
operator-(
    const Capacity<Prec, UseFloat> &c1,
    const Capacity<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) - get(c2);
  return Capacity<Prec, UseFloat>{value};
}
template <typename Prec, typename UseFloat>
constexpr auto
operator+(
    const Capacity<Prec, UseFloat> &c1,
    const Capacity<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) + get(c2);
  return Capacity<Prec, UseFloat>{value};
}

//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_int_tag>
struct Size
  : ts::strong_typedef<Size<Prec, UseFloat>, count_t<Prec, UseFloat>>,
    ts::strong_typedef_op::equality_comparison<Size<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Size<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Size<Prec, UseFloat>>
{
  using ts::strong_typedef<Size<Prec, UseFloat>, count_t<Prec, UseFloat>>::
      strong_typedef;

  // template <
  // typename Prec2,
  // typename =
  // std::enable_if_t<std::is_same_v<UseFloat, use_float_tag>>>
  // constexpr Size(const Size<Prec2, use_int_tag> &c) : Size(get(c))
  // {
  // }
  template <typename Prec2, typename UseFloat2>
  constexpr Size(const Size<Prec2, UseFloat2> &c)
    : Size(count_t<Prec, UseFloat>(get(c)))
  {
  }
  constexpr auto &operator+=(const Size &s)
  {
    return ts::get(*this) += ts::get(s);
  }
  constexpr auto &operator-=(const Size &s)
  {
    return ts::get(*this) -= ts::get(s);
  }

  constexpr bool operator==(const Capacity<Prec> &c)
  {
    return ts::get(*this) == ts::get(c);
  }
  constexpr bool operator<=(const Capacity<Prec> &c)
  {
    return ts::get(*this) <= ts::get(c);
  }
  constexpr bool operator>(const Capacity<Prec> &c)
  {
    return ts::get(*this) > ts::get(c);
  }
  explicit operator Size<Prec, use_int_tag>() const
  {
    return Size<Prec, use_int_tag>{
        static_cast<count_t<Prec, use_int_tag>>(std::ceil(get(*this)))};
  }
};

template <typename Prec, typename UseFloat>
constexpr auto
operator-(const Size<Prec, UseFloat> &c1, const Size<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) - get(c2);
  return Size<Prec, UseFloat>{value};
}
template <typename Prec, typename UseFloat>
constexpr auto
operator+(const Size<Prec, UseFloat> &c1, const Size<Prec, UseFloat> &c2)
{
  count_t<Prec, UseFloat> value = get(c1) + get(c2);
  return Size<Prec, UseFloat>{value};
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
    const Capacity<Prec, CapacityUseFloat> &c,
    const Size<Prec2, SizeUseFloat> &       s)
{
  count_t<Result, CommonUseFloat> value = get(c) - get(s);
  return Capacity<Result, CommonUseFloat>{value};
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
    const Capacity<Prec, CapacityUseFloat> &c,
    const Size<Prec2, SizeUseFloat> &       s)
{
  count_t<Result, CommonUseFloat> value = get(c) + get(s);
  return Capacity<Result, CommonUseFloat>{value};
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
operator>=(
    const Capacity<Prec, use_int_tag> &c,
    const Size<Prec, use_int_tag> &    s)
{
  return get(c) >= get(s);
}
//----------------------------------------------------------------------
template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct IntensitySize
  : ts::strong_typedef<
        IntensitySize<Prec, UseFloat>,
        intensity_t<Prec, UseFloat>>,
    ts::strong_typedef_op::multiplication<IntensitySize<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<IntensitySize<Prec, UseFloat>>
{
  using ts::strong_typedef<
      IntensitySize<Prec, UseFloat>,
      intensity_t<Prec, UseFloat>>::strong_typedef;
};

//----------------------------------------------------------------------

template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct Intensity
  : ts::strong_typedef<Intensity<Prec, UseFloat>, intensity_t<Prec, UseFloat>>,
    ts::strong_typedef_op::multiplication<Intensity<Prec, UseFloat>>,
    ts::strong_typedef_op::addition<Intensity<Prec, UseFloat>>,
    ts::strong_typedef_op::relational_comparison<Intensity<Prec, UseFloat>>,
    ts::strong_typedef_op::output_operator<Intensity<Prec, UseFloat>>
{
  using ts::strong_typedef<
      Intensity<Prec, UseFloat>,
      intensity_t<Prec, UseFloat>>::strong_typedef;
  template <
      typename Prec2,
      typename = std::enable_if<std::is_same_v<precision_t<Prec, Prec2>, Prec>>>
  Intensity(const Intensity<Prec2> &intensity) : Intensity(get(intensity))
  {
  }
  constexpr auto operator/(const Intensity<Prec, UseFloat> &intensity) const
  {
    return Intensity<Prec, UseFloat>{ts::get(*this) / ts::get(intensity)};
  }
  constexpr auto operator/(const Size<Prec> &size) const
  {
    return Intensity<Prec, UseFloat>(ts::get(*this) / ts::get(size));
  }
  template <
      typename Prec2 = mediump,
      typename SizeUseFloat,
      typename Result = precision_t<Prec, Prec2>>
  constexpr auto operator*(const Size<Prec2, SizeUseFloat> &size) const
  {
    typename Result::float_t value = get(*this) * get(size);
    return IntensitySize<Result, promote_t<SizeUseFloat, UseFloat>>(value);
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
  return (get(intensity1) + epsilon) > get(intensity2)
         && (get(intensity1) - epsilon) < get(intensity2);
}

template <typename Prec = mediump, typename UseFloat = use_float_tag>
constexpr auto
operator/(
    const IntensitySize<Prec, UseFloat> &intensity,
    const Size<Prec, UseFloat> &         size)
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
    const Capacity<Prec, CapacityUseFloat> &      size)
{
  return Intensity<Prec, UseFloat>(ts::get(intensity) / ts::get(size));
}
//----------------------------------------------------------------------
template <typename Prec = mediump>
struct Probability
  : ts::strong_typedef<Probability<Prec>, probability_t<Prec>>,
    ts::strong_typedef_op::relational_comparison<Probability<Prec>>,
    ts::strong_typedef_op::equality_comparison<Probability<Prec>>,
    ts::strong_typedef_op::output_operator<Probability<Prec>>
{
  using ts::strong_typedef<Probability<Prec>, probability_t<Prec>>::
      strong_typedef;

  template <typename Prec2, typename UseFloat>
  constexpr Probability &operator/=(const Capacity<Prec2, UseFloat> &capacity)
  {
    ts::get(*this) /= ts::get(capacity);
    return *this;
  }
  constexpr Probability opposite()
  {
    probability_t<Prec> value = 1 - get(*this);
    return Probability{value};
  }
  constexpr Probability &operator/=(const Probability &p)
  {
    get(*this) /= get(p);
    return *this;
  }
  constexpr Probability &operator+=(const Probability &p)
  {
    get(*this) += get(p);
    return *this;
  }

  template <typename Prec2>
  explicit constexpr Probability(const Ratio<Prec2> &ratio)
  {
    get(*this) = get(ratio);
  }
};

template <typename Prec>
constexpr auto
operator+(const Probability<Prec> &p1, const Probability<Prec> &p2)
{
  probability_t<Prec> value = get(p1) + get(p2);
  return Probability<Prec>{value};
}

template <typename Prec>
constexpr auto
operator-(const Probability<Prec> &p1, const Probability<Prec> &p2)
{
  probability_t<Prec> value = get(p1) - get(p2);
  return Probability<Prec>{value};
}

template <typename Prec>
constexpr auto
operator*(const Probability<Prec> &p1, const Probability<Prec> &p2)
{
  probability_t<Prec> value = get(p1) * get(p2);
  return Probability<Prec>{value};
}

template <typename Prec>
constexpr auto
operator/(const Probability<Prec> &p1, const Probability<Prec> &p2)
{
  probability_t<Prec> value = get(p1) / get(p2);
  return Probability<Prec>{value};
}

template <typename Prec = mediump>
constexpr auto
operator/(const Probability<Prec> &probability, const Capacity<Prec> &capacity)
{
  return Probability<Prec>{get(probability) / get(capacity)};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename UseFloat,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const Weight<Prec, UseFloat> &weight,
    const Probability<Prec2> &    probability)
{
  probability_t<Result> result = get(weight) * get(probability);
  return Probability<Result>{result};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const IntensitySize<Prec> &intensity,
    const Probability<Prec2> & probability)
{
  probability_t<Result> result = get(intensity) * get(probability);
  return Probability<Result>{result};
}

template <typename Prec = mediump, typename UseFloat = use_float_tag>
struct IntensityFactor
  : ts::strong_typedef<IntensityFactor<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::multiplication<IntensityFactor<Prec>>,
    ts::strong_typedef_op::output_operator<IntensityFactor<Prec>>
{
  using ts::strong_typedef<IntensityFactor<Prec>, intensity_t<Prec>>::
      strong_typedef;

  constexpr auto operator*(const Intensity<Prec> &intensity) const
  {
    return Intensity<Prec>(ts::get(*this) * ts::get(intensity));
  }
};

template <typename Prec = mediump>
constexpr auto
operator/(const Size<Prec> &size, const IntensityFactor<Prec> &intensity_factor)
{
  return Size<Prec>{
      static_cast<count_t<Prec>>(get(size) / get(intensity_factor))};
}

//----------------------------------------------------------------------

template <typename Prec = mediump>
constexpr auto
operator*(const Count<Prec> &count, const Intensity<Prec> &intensity)
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
  ratio_t<Prec> result = static_cast<ratio_t<Prec>>(ts::get(c1))
                         / static_cast<ratio_t<Prec>>(ts::get(c2));
  return Ratio<Prec>{result};
}

template <typename Prec>
constexpr auto operator*(const Count<Prec> &count, const Count<Prec> &count2)
{
  return Count<Prec>{get(count2) * get(count)};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename CapacityUseFloat,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator*(const Count<Prec> &count, const Capacity<Prec2, CapacityUseFloat> &c)
{
  count_t<Result, CapacityUseFloat> value = get(c) * get(count);
  return Capacity<Result, CapacityUseFloat>{value};
}

//----------------------------------------------------------------------
template <typename Prec = mediump>
struct Duration : ts::strong_typedef<Duration<Prec>, duration_t<Prec>>,
                  ts::strong_typedef_op::equality_comparison<Duration<Prec>>,
                  ts::strong_typedef_op::subtraction<Duration<Prec>>,
                  ts::strong_typedef_op::addition<Duration<Prec>>,
                  ts::strong_typedef_op::output_operator<Duration<Prec>>
{
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
                  ts::strong_typedef_op::output_operator<Variance<Prec>>
{
  using ts::strong_typedef<Variance<Prec>, stat_t<Prec>>::strong_typedef;
  template <typename Prec2>
  explicit Variance(const Intensity<Prec2> &intensity)
    : Variance(get(intensity))
  {
  }
};

//----------------------------------------------------------------------

template <typename Prec = mediump>
struct MeanRequestNumber
  : ts::strong_typedef<MeanRequestNumber<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::addition<MeanRequestNumber<Prec>>,
    ts::strong_typedef_op::output_operator<MeanRequestNumber<Prec>>
{
  using ts::strong_typedef<MeanRequestNumber<Prec>, intensity_t<Prec>>::
      strong_typedef;
};

//----------------------------------------------------------------------

template <typename Prec = mediump>
struct MeanIntensity
  : ts::strong_typedef<MeanIntensity<Prec>, intensity_t<Prec>>,
    ts::strong_typedef_op::addition<MeanIntensity<Prec>>,
    ts::strong_typedef_op::output_operator<MeanIntensity<Prec>>
{
  using ts::strong_typedef<MeanIntensity<Prec>, intensity_t<Prec>>::
      strong_typedef;
  explicit operator MeanRequestNumber<Prec>()
  {
    return MeanRequestNumber<Prec>{get(*this)};
  }
  template <
      typename Prec2,
      typename = std::enable_if<std::is_same_v<precision_t<Prec, Prec2>, Prec>>>
  MeanIntensity(const Intensity<Prec2> &intensity)
    : MeanIntensity(get(intensity))
  {
  }
};
//----------------------------------------------------------------------

template <typename Prec = mediump>
struct Peakedness
  : ts::strong_typedef<Peakedness<Prec>, stat_t<Prec>>,
    ts::strong_typedef_op::relational_comparison<Peakedness<Prec>>,
    ts::strong_typedef_op::output_operator<Peakedness<Prec>>
{
  using ts::strong_typedef<Peakedness<Prec>, stat_t<Prec>>::strong_typedef;
};

template <typename Prec>
constexpr auto
operator+(const Peakedness<Prec> &w1, const Peakedness<Prec> &w2)
{
  stat_t<Prec> value = get(w1) + get(w2);
  return Peakedness<Prec>{value};
}

template <typename Prec = mediump>
constexpr auto operator
    *(const Variance<Prec> &variance, const Weight<Prec, use_float_tag> &weight)
{
  stat_t<Prec> value = get(variance) * get(weight);
  return Peakedness<Prec>{value};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator/(const Variance<Prec> &variance, const MeanIntensity<Prec2> &mean)
    -> Peakedness<Result>
{
  typename Result::float_t value = get(variance) / get(mean);
  return Peakedness<Result>{value};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator/(const MeanIntensity<Prec> &mean, const Peakedness<Prec2> &peakedness)
    -> Intensity<Result>
{
  typename Result::float_t value = get(mean) / get(peakedness);
  return Intensity<Result>{value};
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const Intensity<Prec> &   intensity,
    const Probability<Prec2> &probability) -> MeanIntensity<Result>
{
  typename Result::float_t value = get(intensity) * get(probability);
  return MeanIntensity<Result>(value);
}

template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename UseFloat,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator/(
    const Capacity<Prec, UseFloat> &capacity,
    const Peakedness<Prec2> &peakedness) -> Capacity<Result, use_float_tag>
{
  count_t<Result, use_float_tag> result =
      static_cast<stat_t<Prec>>(get(capacity)) / get(peakedness);
  return Capacity<Result, use_float_tag>{result};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator*(const MeanIntensity<Prec> &mean, const Size<Prec2> &size)
    -> Weight<Result, use_float_tag>
{
  typename Result::float_t value = get(mean) * get(size);
  return Weight<Result, use_float_tag>{value};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto
operator*(const MeanRequestNumber<Prec> &mean, const Size<Prec2> &size)
{
  count_t<Result, use_float_tag> result = get(mean) * get(size);
  return Capacity<Result, use_float_tag>{result};
}
template <
    typename Prec = mediump,
    typename Prec2 = mediump,
    typename Result = precision_t<Prec, Prec2>>
constexpr auto operator*(
    const MeanRequestNumber<Prec> &   mean,
    const Size<Prec2, use_float_tag> &size)
{
  count_t<Result, use_float_tag> value = get(mean) * get(size);
  return Capacity<Result, use_float_tag>{value};
}
template <
    typename PrecSize = mediump,
    typename PrecWeight = mediump,
    typename Result = precision_t<PrecSize, PrecWeight>>
constexpr auto operator*(
    const Size<PrecSize> &                      size,
    const InvWeight<PrecWeight, use_float_tag> &inv_weight)
{
  weight_t<Result, use_float_tag> value = get(size) * get(inv_weight);
  return Weight<Result, use_float_tag>{value};
}
//----------------------------------------------------------------------

template <typename Prec = mediump>
struct SizeRescale : ts::strong_typedef<SizeRescale<Prec>, intensity_t<Prec>>,
                     ts::strong_typedef_op::output_operator<SizeRescale<Prec>>
{
  using ts::strong_typedef<SizeRescale<Prec>, intensity_t<Prec>>::
      strong_typedef;
  explicit SizeRescale(const Peakedness<Prec> &peakedness)
    : SizeRescale(get(peakedness))
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
    const SizeRescale<Prec> &           rescale,
    const Size<PrecSize, SizeUseFloat> &size)
{
  count_t<Result, use_float_tag> value = get(rescale) * get(size);
  return Size<Result, use_float_tag>{value};
}
//----------------------------------------------------------------------

template <typename Prec = mediump>
struct Time : ts::strong_typedef<Time<Prec>, time_type<Prec>>,
              ts::strong_typedef_op::equality_comparison<Time<Prec>>,
              ts::strong_typedef_op::relational_comparison<Time<Prec>>,
              ts::strong_typedef_op::output_operator<Time<Prec>>
{
  using ts::strong_typedef<Time<Prec>, time_type<Prec>>::strong_typedef;
  explicit constexpr operator Duration<Prec>() const
  {
    return Duration<Prec>(ts::get(*this));
  }
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
