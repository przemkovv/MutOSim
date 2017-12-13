#pragma once

#include "types.h"

namespace Math
{
int64_t factorial(int64_t k);

template <typename T>
T ratio_to_sum(T s1, T s2)
{
  return s1 / (s1 + s2);
}

} // namespace Math
