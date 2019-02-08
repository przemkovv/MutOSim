
#include "model/group.h"

#include "logger.h"
#include "overflow_far.h"
#include "traffic_class.h"

#include <range/v3/view/map.hpp>
namespace rng = ranges;

namespace Model
{
const OutgoingRequestStreams &
Group::get_outgoing_request_streams() const
{
  if (need_recalculate_) {
    IncomingRequestStreams in_request_streams = in_request_streams_ | rng::view::values;

    const auto V = [&]() {
      if (kr_variant_ == KaufmanRobertsVariant::FixedCapacity) {
        return CapacityF{total_V_};
      } else {
        const auto peakedness = compute_collective_peakedness(in_request_streams);
        return total_V_ / peakedness;
      }
    }();

    out_request_streams_ = kaufman_roberts_blocking_probability(in_request_streams, V, kr_variant_);

    out_request_streams_ = compute_overflow_parameters(out_request_streams_, V);

    need_recalculate_ = false;
  }
  return out_request_streams_;
}

Group::Group(std::vector<Capacity> V, KaufmanRobertsVariant kr_variant)
  : V_(V), kr_variant_(kr_variant)
{
}

Group::Group(Capacity V, KaufmanRobertsVariant kr_variant) : V_({V}), kr_variant_(kr_variant)
{
}

} // namespace Model
