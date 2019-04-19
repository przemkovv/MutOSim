
#pragma once

#include "config.h"

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
// #include <boost/multiprecision/gmp.hpp>
// #include <boost/multiprecision/mpfr.hpp>
#include <string>
#include <type_traits>

namespace mp = boost::multiprecision;

constexpr long double epsilon = 0.00000001L;

struct use_float_tag;
struct use_int_tag;

struct mediump
{
  using float_t = long double;
  using int_t = int64_t;
};

struct highp
{
  static constexpr auto digits_number = 50;
  using float_t = mp::number<mp::cpp_dec_float<digits_number>>;
  using int_t = mp::int128_t;
};

/*
struct highp_float
{
  static constexpr auto digit_number = 50;
  using float_t = std::conditional_t<
      Config::debug_mpfr,
      mp::number<mp::debug_adaptor<mp::mpfr_float_backend<digit_number>>>,
      mp::number<mp::mpfr_float_backend<digit_number>>>;
  using int_t = mp::mpz_int;
};
*/

template <typename Precision, typename UseFloatTag>
using PrecisionType = std::conditional_t<
    std::is_same_v<UseFloatTag, use_float_tag>,
    typename Precision::float_t,
    typename Precision::int_t>;

using uuid_t = uint64_t;
using name_t = std::string;

using OverflowPolicyName = name_t;
using Uuid = uuid_t;
using Name = name_t;
using Layer = uint64_t;
using Length = uint64_t;

constexpr auto  MaxPathLength = std::numeric_limits<Length>::max();
constexpr Layer MaxLayersNumber = 3;

template <typename Precision = mediump, typename UseFloat = use_float_tag>
using time_type = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump, typename UseFloat = use_float_tag>
using duration_t = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump, typename UseFloat = use_int_tag>
using count_t = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump, typename UseFloat = use_float_tag>
using count_float_t = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump, typename UseFloat = use_float_tag>
using stat_t = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump, typename UseFloat = use_float_tag>
using probability_t = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump>
using intensity_t = PrecisionType<Precision, use_float_tag>;

template <typename Precision = mediump, typename UseFloat = use_int_tag>
using weight_t = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump, typename UseFloat = use_float_tag>
using weight_float_t = PrecisionType<Precision, UseFloat>;

template <typename Precision = mediump>
using ratio_t = PrecisionType<Precision, use_float_tag>;

template <typename Precision = mediump, typename UseFloat = use_int_tag>
using threshold_t = count_t<Precision, UseFloat>;
