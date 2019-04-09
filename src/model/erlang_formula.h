
#pragma once

#include "stream_properties.h"
#include "types/types.h"

#include <optional>

std::optional<Model::CapacityF>
compute_fictitious_capacity_fit_blocking_probability(
    const Model::OutgoingRequestStream &rs,
    Model::CapacityF                    V);
