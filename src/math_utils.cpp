
#include "math_utils.h"

#include <boost/math/special_functions.hpp>

namespace Math {
int64_t
n_over_k(const int64_t n, const int64_t k)
{
  if (n < 0)
  {
    return n_over_k(-n + k - 1, k);
  }
  return static_cast<int64_t>(boost::math::binomial_coefficient<double>(
      static_cast<uint32_t>(n), static_cast<uint32_t>(k)));
}

highp::int_t
n_over_k(const highp::int_t &n, const highp::int_t &k)
{
  return n_over_k(int64_t(n), int64_t(k));
}
} // namespace Math
