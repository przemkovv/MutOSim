#pragma once

#include "stream_properties.h"
#include "traffic_class.h"
#include "types.h"

#include <fmt/ostream.h>
#include <valarray>
#include <vector>

namespace Model
{
using Probabilities = std::valarray<Probability>;

Probabilities kaufman_roberts_distribution(
    const IncomingRequestStreams &in_request_streams,
    Capacity V,
    SizeRescale size_rescale);

OutgoingRequestStreams kaufman_roberts_blocking_probability(
    const IncomingRequestStreams &in_request_streams,
    CapacityF V,
    SizeRescale size_rescale);

OutgoingRequestStreams compute_overflow_parameters(
    OutgoingRequestStreams out_request_streams, CapacityF V, SizeRescale size_rescale);

//----------------------------------------------------------------------
IncomingRequestStreams convert_to_incoming_streams(
    const std::vector<OutgoingRequestStreams> &out_request_streams_per_group);

//----------------------------------------------------------------------
Peakedness
compute_collective_peakedness(const IncomingRequestStreams &in_request_streams);

Variance compute_riordan_variance(
    MeanIntensity mean, Intensity intensity, CapacityF fictional_capacity, SizeF tc_size);

CapacityF compute_fictional_capacity_fit_carried_traffic(
    const OutgoingRequestStreams &request_streams,
    Capacity V,
    TrafficClassId tc_id,
    SizeRescale size_rescale);

CapacityF compute_fictional_capacity_fit_blocking_probability(
    const OutgoingRequestStream &rs, CapacityF V, SizeRescale size_rescale);

Count combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f);

Probability transition_probability(
    Capacity n, Capacity V, Count resources_number, Capacity f, Size t);

} // namespace Model
