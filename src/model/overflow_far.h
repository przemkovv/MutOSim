#pragma once

#include "model/common.h"
#include "resource.h"
#include "stream_properties.h"
#include "stream_properties_format.h"
#include "traffic_class.h"
#include "types/types.h"

#include <valarray>
#include <vector>

namespace Model {
using Probabilities = std::vector<Probability>;

Probabilities kaufman_roberts_distribution(
    const IncomingRequestStreams &in_request_streams,
    Resource<>                    resource,
    Size                          offset,
    KaufmanRobertsVariant         kr_variant);

OutgoingRequestStreams kaufman_roberts_blocking_probability(
    const IncomingRequestStreams &in_request_streams,
    Resource<CapacityF>           resource,
    KaufmanRobertsVariant         kr_variant);

OutgoingRequestStreams
compute_overflow_parameters(OutgoingRequestStreams out_request_streams, CapacityF V);

//----------------------------------------------------------------------
IncomingRequestStreams convert_to_incoming_streams(
    const std::vector<OutgoingRequestStreams> &out_request_streams_per_group);

//----------------------------------------------------------------------
Peakedness compute_collective_peakedness(const IncomingRequestStreams &in_request_streams);

Variance compute_riordan_variance(
    MeanIntensity mean,
    Intensity     intensity,
    CapacityF     fictitous_capacity,
    SizeF         tc_size);

CapacityF compute_fictitious_capacity_fit_carried_traffic(
    const OutgoingRequestStreams &request_streams,
    CapacityF                     V,
    TrafficClassId                tc_id,
    KaufmanRobertsVariant         kr_variant);

// Count combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f);
Count combinatorial_arrangement_number(Capacity x, const ResourceComponent<> &component);

Probability
conditional_transition_probability(Capacity n, const ResourceComponent<> &component, Size t);

Count combinatorial_arrangement_number_unequal_resources(
    Capacity /*x*/,
    std::vector<Count> components_numbers,
    std::vector<Capacity> /*components_capacities*/);

} // namespace Model
