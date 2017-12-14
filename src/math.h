#pragma once

#include "types.h"

namespace Math
{
template <typename T>
T ratio_to_sum(T s1, T s2)
{
  return s1 / (s1 + s2);
}

constexpr double pow(double x, int64_t y)
{
  return y == 0 ? 1.0 : x * pow(x, y - 1);
}

constexpr int64_t factorial(int64_t x)
{
  return x == 0 ? 1 : x * factorial(x - 1);
}

constexpr double exp(double x)
{
  return 1.0 + x + pow(x, 2) / factorial(2) + pow(x, 3) / factorial(3) +
         pow(x, 4) / factorial(4) + pow(x, 5) / factorial(5) +
         pow(x, 6) / factorial(6) + pow(x, 7) / factorial(7) +
         pow(x, 8) / factorial(8) + pow(x, 9) / factorial(9);
}

constexpr double power_over_factorial(const double x, const int64_t i)
{
  return Math::pow(x, i) / Math::factorial(i);
}

constexpr int64_t product(int64_t from, int64_t to)
{
  int64_t result = 1;
  for (; from <= to; ++from) {
    result *= from;
  }
  return result;
}

constexpr int64_t n_over_k(const int64_t n, const int64_t k)
{
  if (k > n - k) {
    return product(k + 1, n) / factorial(n - k);
  } else {
    return product(n - k + 1, n) / factorial(k);
  }
}

} // namespace Math
