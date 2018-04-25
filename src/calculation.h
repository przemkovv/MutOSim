
#pragma once

#include "math_utils.h"

#include <types.h>

inline auto erlang_pk_distribution(const long double A, const int64_t V, const int64_t k)
{
  const auto dividend = Math::power_over_factorial(A, k);
  auto divisor = 0.0L;
  for (int64_t i = 0; i <= V; ++i) {
    divisor += Math::power_over_factorial(A, i);
  }
  return dividend / divisor;
}

inline auto erlang_block_probability(const long double A, const int64_t V)
{
  return erlang_pk_distribution(A, V, V);
}

inline auto
engset_pi(const long double alpha, const int64_t V, const int64_t N, const int64_t i)
{
  if (N == 0)
    return 0.0L;
  const auto dividend = Math::n_over_k(N, i) * Math::pow(alpha, i);
  auto divisor = 0L;
  for (int64_t j = 0; j <= V; ++j) {
    divisor += Math::n_over_k(N, j) * Math::pow(alpha, j);
  }
  return dividend / divisor;
}
