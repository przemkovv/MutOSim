#pragma once

#include "stream_properties.h"
#include "traffic_class.h"
#include "types.h"

#include <fmt/ostream.h>
#include <vector>

namespace Model
{
using Probabilities = std::vector<Probability>;

Probabilities
KaufmanRobertsDistribution(const std::vector<IncomingRequestStream> &streams, Capacity V);

std::vector<OutgoingRequestStream>
KaufmanRobertsBlockingProbability(std::vector<IncomingRequestStream> &streams, Capacity V);

//----------------------------------------------------------------------
std::vector<IncomingRequestStream> convert_to_overflowing_streams(
    const std::vector<std::vector<OutgoingRequestStream>> &request_streams_per_group);

//----------------------------------------------------------------------
Peakness compute_collective_peakness(const std::vector<IncomingRequestStream> &streams);

Variance compute_riordan_variance(
    MeanIntensity mean, Intensity intensity, CapacityF fictional_capacity, Size tc_size);
CapacityF compute_fictional_capacity(
    const std::vector<OutgoingRequestStream> &request_streams, Capacity V, TrafficClassId tc_id);
Count combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f);

Probability transition_probability(
    Capacity n, Capacity V, Count resources_number, Capacity f, Size t);

} // namespace Model
