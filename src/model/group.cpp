
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
    const auto peakedness = compute_collective_peakedness(in_request_streams);

    const auto [V, size_rescale] = [&]() {
      if (assume_fixed_capacity_) {
        return std::make_pair(CapacityF{V_}, SizeRescale{peakedness});
      } else {
        return std::make_pair(V_ / peakedness, SizeRescale{1});
      }
    }();

    out_request_streams_ =
        kaufman_roberts_blocking_probability(in_request_streams, V, size_rescale);

    out_request_streams_ =
        compute_overflow_parameters(out_request_streams_, V, size_rescale);

    need_recalculate_ = false;
  }
  return out_request_streams_;
}

Group::Group(Capacity V, bool assume_fixed_capacity)
  : V_(V), assume_fixed_capacity_(assume_fixed_capacity)
{
}

} // namespace Model
