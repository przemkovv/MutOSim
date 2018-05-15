#pragma once

namespace Model
{
enum class AnalyticModel { KaufmanRobertsFixedCapacity, KaufmanRobertsFixedReqSize };
enum class KaufmanRobertsVariant { FixedCapacity, FixedReqSize };

enum class LayerType {
  FullAvailability,
  DistributedEqualCapacities,
  DistributedUnequalCapacities,
  Unknown
};
} // namespace Model
