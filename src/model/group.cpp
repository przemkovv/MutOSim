
#include "model/group.h"

#include "overflow_far.h"
#include "traffic_class.h"

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/view/map.hpp>
#include <type_traits>
namespace rng = ranges;

namespace Model
{
const std::vector<OutgoingRequestStream> &
Group::get_outgoing_request_streams() const
{
  if (need_recalculate_) {
    std::vector<IncomingRequestStream> in_request_streams = in_request_streams_ |
                                                            rng::view::values;
    auto peakness = compute_collective_peakness(in_request_streams);
    out_request_streams_ =
        KaufmanRobertsBlockingProbability(in_request_streams, V_, peakness, true);
    need_recalculate_ = false;
  }
  return out_request_streams_;
}

Group::Group(Capacity V) : V_(V)
{
}

} // namespace Model
