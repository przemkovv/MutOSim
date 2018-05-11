#pragma once

#include "stream_properties.h"
#include "traffic_class.h"
#include "types.h"

#include <fmt/ostream.h>
#include <vector>

namespace Model
{
using Probabilities = std::vector<Probability>;

Probabilities KaufmanRobertsDistribution(
    const std::vector<IncomingRequestStream> &in_request_streams,
    Capacity V,
    Peakness size_rescale);

std::vector<OutgoingRequestStream> KaufmanRobertsBlockingProbability(
    std::vector<IncomingRequestStream> &in_request_streams,
    Capacity V,
    Peakness peakness,
    bool fixed_capacity);

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
    Peakness size_rescale);

Probability extended_erlang_b(CapacityF V, Intensity A);
CapacityF compute_fictional_capacity_fit_blocking_probability(
    const OutgoingRequestStream &rs, Capacity V, Peakness size_rescale);

Count combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f);

Probability transition_probability(
    Capacity n, Capacity V, Count resources_number, Capacity f, Size t);

} // namespace Model
