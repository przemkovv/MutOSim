
#pragma once

#include "math.h"

#include <types.h>

constexpr double erlang_pk(const double A, const int64_t V, const int64_t k)
{
  const double dividend = Math::power_over_factorial(A, k);
  double divisor = 0;
  for (int64_t i = 0; i <= V; ++i) {
    divisor += Math::power_over_factorial(A, i);
  }
  return dividend / divisor;
}

constexpr double
engset_pi(const double alpha, const int64_t V, const int64_t N, const int64_t i)
{
  if (N==0 ) return 0;
  const double dividend = Math::n_over_k(N, i) * Math::pow(alpha, i);
  double divisor = 0;
  for (int64_t j = 0; j <= V; ++j) {
    divisor += Math::n_over_k(N, j) * Math::pow(alpha, j);
  }
  return dividend / divisor;
}
