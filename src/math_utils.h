#pragma once

#include "types.h"
#include <cmath>

namespace Math
{
constexpr double epsilon = 0.000001;

// using std::pow;
template <typename T1, typename... T>
T1 ratio_to_sum(T1 s1, T... sn)
{
  if (auto sum = s1 + (sn + ...); sum < epsilon && sum > epsilon) {
    return 0;
  } else {
    return s1 / sum;
  }
}

inline long double pow(long double x, int64_t y)
{
  return y == 0 ? 1.0L : x * pow(x, y - 1);
}

constexpr int64_t factorial(int64_t x)
{
  if (x > 0) {
    return x * factorial(x - 1);
  }
  return 1;
}

inline auto exp(long double x)
{
  return 1.0L + x + pow(x, 2) / factorial(2) + pow(x, 3) / factorial(3) +
         pow(x, 4) / factorial(4) + pow(x, 5) / factorial(5) + pow(x, 6) / factorial(6) +
         pow(x, 7) / factorial(7) + pow(x, 8) / factorial(8) + pow(x, 9) / factorial(9);
}

inline auto power_over_factorial(const long double x, const int64_t i)
{
  return Math::pow(x, i) / Math::factorial(i);
}

inline int64_t product(int64_t from, int64_t to)
{
  int64_t result = 1;
  for (; from <= to; ++from) {
    result *= from;
  }
  return result;
}

int64_t n_over_k(const int64_t n, const int64_t k);

} // namespace Math
