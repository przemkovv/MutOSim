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
    const std::vector<IncomingRequestStream> &in_request_streams,
    Capacity V,
    SizeRescale size_rescale);

std::vector<OutgoingRequestStream> kaufman_roberts_blocking_probability(
    const std::vector<IncomingRequestStream> &in_request_streams,
    CapacityF V,
    SizeRescale size_rescale);

std::vector<OutgoingRequestStream> compute_overflow_parameters(
    std::vector<OutgoingRequestStream> out_request_streams,
    CapacityF V,
    SizeRescale size_rescale);

//----------------------------------------------------------------------
std::vector<IncomingRequestStream> convert_to_incoming_streams(
    const std::vector<std::vector<OutgoingRequestStream>> &out_request_streams_per_group);

//----------------------------------------------------------------------
Peakness
compute_collective_peakness(const std::vector<IncomingRequestStream> &in_request_streams);

Variance compute_riordan_variance(
    MeanIntensity mean, Intensity intensity, CapacityF fictional_capacity, SizeF tc_size);

CapacityF compute_fictional_capacity_fit_carried_traffic(
    const std::vector<OutgoingRequestStream> &request_streams,
    Capacity V,
    TrafficClassId tc_id,
    SizeRescale size_rescale);

CapacityF compute_fictional_capacity_fit_blocking_probability(
    const OutgoingRequestStream &rs, CapacityF V, SizeRescale size_rescale);

Count combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f);

Probability transition_probability(
    Capacity n, Capacity V, Count resources_number, Capacity f, Size t);

} // namespace Model
