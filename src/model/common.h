#pragma once

namespace Model
{
enum class AnalyticModel { KaufmanRobertsFixedCapacity, KaufmanRobertsFixedReqSize };

enum class LayerType {
  FullAvailability,
  DistributedEqualCapacities,
  DistributedUnequalCapacities,
  Unknown
};
} // namespace Model
