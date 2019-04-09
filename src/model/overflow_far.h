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
    Resource<CapacityF>           resource,
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

template <typename C>
Count combinatorial_arrangement_number(Capacity x, const ResourceComponent<C> &component);

template <typename C>
Probability
conditional_transition_probability(Capacity n, const ResourceComponent<C> &component, Size t);

template <typename C>
Count combinatorial_arrangement_number_unequal_resources(Capacity x, Resource<C> resource);
template <typename C>
Probability conditional_transition_probability(Capacity n, const Resource<C> &resource, Size t);

} // namespace Model
