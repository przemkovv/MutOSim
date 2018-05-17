
#include "calculation.h"

#include "logger.h"

#include <boost/math/special_functions/gamma.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

using namespace boost::multiprecision;
using namespace boost::math;
using float_hp = number<cpp_dec_float<50>>;

float_hp erlang_b(float_hp V, float_hp A);
float_hp erlang_b_inv(float_hp V, float_hp A);

float_hp
erlang_b(float_hp V, float_hp A)
{
  float_hp erlang = 1;
  for (float_hp x = 1; x <= V; x += 1) {
    erlang = (A * erlang) / (x + A * erlang);
  }
  return erlang;
}
float_hp
erlang_b_inv(float_hp V, float_hp A)
{
  float_hp inv_erlang = 1;
  for (float_hp x = 1; x <= V; x += 1) {
    inv_erlang = 1 + x / A * inv_erlang;
  }

  return 1 / inv_erlang;
}
Probability
extended_erlang_b(CapacityF V, Intensity A)
{
  float_hp x = get(V);
  float_hp a = get(A);

  return Probability{probability_t{pow(a, x) * exp(-a) / tgamma(x + 1, a)}};
}

CapacityF
compute_fictional_capacity_fit_blocking_probability2(
    const Model::OutgoingRequestStream &rs, CapacityF V)
{
  float_hp target_p_block = get(rs.blocking_probability);
  float_hp a = get(rs.intensity);
  float_hp e = 1e-20;
  float_hp left_bound = 1;
  float_hp right_bound = get(V);
  float_hp current = left_bound;

  while (right_bound - left_bound > e) {
    current = (right_bound + left_bound) * float_hp{0.5L};
    float_hp p = pow(a, current) * exp(-a) / tgamma(current + 1, a);
    // print("{} {} {}\t", pow(a, current), exp(-a), tgamma(current + 1, a));
    // println(
    // "int={} <{}; {}; {}> {} - {} = {}",
    // a,
    // left_bound,
    // current,
    // right_bound,
    // count_float_t{p},
    // rs.blocking_probability,
    // p - target_p_block);
    if (p > target_p_block) {
      left_bound = current;
    } else if (p < target_p_block) {
      right_bound = current;
    } else {
      break;
    }
  }

  // TODO(PW): check if multiply by peakness is needed
  return CapacityF{count_float_t{current * get(rs.tc.size)}};
}

