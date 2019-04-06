
#pragma once

#include "types/types.h"

#include <vector>

namespace Model {
using Probabilities = std::vector<Probability>;

Probabilities kaufman_roberts_distribution(Intensity A, Size tc_size, Capacity V);

Probability kaufman_roberts_blocking_probability(Intensity A, Size tc_size, CapacityF V);

} // namespace Model
