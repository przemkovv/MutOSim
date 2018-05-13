
#include "calculation.h"

#include <boost/math/special_functions/gamma.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

Probability
extended_erlang_b(CapacityF V, Intensity A)
{
  using namespace boost::multiprecision;
  using namespace boost::math;
  number<cpp_dec_float<30>> x = get(V);
  number<cpp_dec_float<30>> a = get(A);

  return Probability{probability_t{pow(a, x) * exp(-a) / tgamma(x + 1, a)}};
}
