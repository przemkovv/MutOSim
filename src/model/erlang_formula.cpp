
#include "erlang_formula.h"

#include <boost/math/special_functions/gamma.hpp>

using namespace boost::math;

//----------------------------------------------------------------------
float_hp
erlang_b(float_hp V, float_hp A)
{
  float_hp erlang = 1;
  for (float_hp x = 1; x <= V; x += 1) {
    erlang = (A * erlang) / (x + A * erlang);
  }
  return erlang;
}

//----------------------------------------------------------------------
float_hp
erlang_b_inv(float_hp V, float_hp A)
{
  float_hp inv_erlang = 1;
  for (float_hp x = 1; x <= V; x += 1) {
    inv_erlang = 1 + x / A * inv_erlang;
  }

  return 1 / inv_erlang;
}

//----------------------------------------------------------------------
float_hp
extended_erlang_b(float_hp V, float_hp A)
{
  return pow(A, V) * exp(-A) / tgamma(V + 1, A);
}

//----------------------------------------------------------------------
// criterion based on blocking probability fit (Formula 3.10)
CapacityF
compute_fictitious_capacity_fit_blocking_probability(
    const Model::OutgoingRequestStream &rs, CapacityF V)
{
  float_hp target_p_block = get(rs.blocking_probability);
  float_hp a = get(rs.intensity);
  float_hp e = 1e-10;
  float_hp left_bound = 0;
  float_hp right_bound = get(V);
  float_hp current = left_bound;

  while (right_bound - left_bound > e) {
    current = (right_bound + left_bound) * float_hp{0.5L};
    float_hp p = extended_erlang_b(current, a);
    if (p > target_p_block) {
      left_bound = current;
    } else if (p < target_p_block) {
      right_bound = current;
    } else {
      break;
    }
  }

  return CapacityF{count_float_t{current * get(rs.tc.size)}};
}
