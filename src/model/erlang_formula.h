
#include "stream_properties.h"
#include "types/types.h"

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <optional>

using float_hp = highp::float_t;

float_hp erlang_b(float_hp V, float_hp A);
float_hp erlang_b_inv(float_hp V, float_hp A);
float_hp extended_erlang_b(float_hp V, float_hp A);

std::optional<CapacityF> compute_fictitious_capacity_fit_blocking_probability(
    const Model::OutgoingRequestStream &rs,
    CapacityF                           V);
