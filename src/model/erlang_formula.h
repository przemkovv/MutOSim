
#include "stream_properties.h"
#include "types.h"

#include <boost/multiprecision/cpp_dec_float.hpp>

using float_hp = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<50>>;

float_hp erlang_b(float_hp V, float_hp A);
float_hp erlang_b_inv(float_hp V, float_hp A);
float_hp extended_erlang_b(float_hp V, float_hp A);

CapacityF compute_fictitious_capacity_fit_blocking_probability(
    const Model::OutgoingRequestStream &rs, CapacityF V);
