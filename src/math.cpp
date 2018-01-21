
#include "math.h"

#include <boost/math/special_functions.hpp>

namespace Math
{
int64_t n_over_k(const int64_t n, const int64_t k)
{
  if (n < 0) {
    return n_over_k(-n + k - 1, k);
  }
  return static_cast<int64_t>(boost::math::binomial_coefficient<double>(
      static_cast<uint32_t>(n), static_cast<uint32_t>(k)));
}
} // namespace Math
