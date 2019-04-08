#pragma once

namespace Model {
enum class AnalyticModel { KaufmanRobertsFixedCapacity, KaufmanRobertsFixedReqSize };
enum class KaufmanRobertsVariant { FixedCapacity, FixedReqSize };

enum class LayerType {
  // All resources are available at any time by each request.
  FullAvailability,

  // Resource is divided to a number of equal components. Only resources from a single component can
  // be reserved by a single request.
  DistributedEqualCapacities,

  // Resource is divided to a number of any size components. Only resources from a single component
  // can be allocated by a single request.
  DistributedUnequalCapacities,

  // Unknown, not supported layer type.
  Unknown
};

} // namespace Model
