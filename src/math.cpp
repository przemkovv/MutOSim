
#include "math.h"

namespace Math
{
int64_t factorial(int64_t k)
{
  int64_t result = 1;
  while (k-- != 0) {
    result *= k;
  }
  return result;
}
} // namespace Math
