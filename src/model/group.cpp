
#include "model/group.h"

#include "logger.h"
#include "overflow_far.h"
#include "resource_format.h"
#include "traffic_class.h"

#include <range/v3/view/map.hpp>
namespace rng = ranges;

namespace Model {
const OutgoingRequestStreams &
Group::get_outgoing_request_streams() const
{
  if (need_recalculate_)
  {
    auto in_request_streams = in_request_streams_ | rng::views::values
                              | rng::to<IncomingRequestStreams>();

    debug_println(
        fg(fmt::color::blue),
        "[Group::get_outgoung_request_streams] {}",
        in_request_streams);
    const auto resource = [&]() {
      if (kr_variant_ == KaufmanRobertsVariant::FixedCapacity)
      {
        return Resource<CapacityF>(resource_);
      }
      else
      {
        const auto peakedness =
            compute_collective_peakedness(in_request_streams);
        debug_println("Collective peakedness: {}", peakedness);
        return resource_ / peakedness;
      }
    }();
    debug_println("[Group::get_outgoung_request_streams] {}", resource);
    auto V = resource.V();

    out_request_streams_ = kaufman_roberts_blocking_probability(
        in_request_streams, resource, kr_variant_);

    out_request_streams_ = compute_overflow_parameters(out_request_streams_, V);

    need_recalculate_ = false;
  }
  return out_request_streams_;
}

Group::Group(std::vector<Capacity> V, KaufmanRobertsVariant kr_variant)
  : resource_(V), kr_variant_(kr_variant)
{
  debug_println("Group resource {}", resource_);
}

Group::Group(Capacity V, KaufmanRobertsVariant kr_variant)
  : resource_({V}), kr_variant_(kr_variant)
{
  debug_println("Group resource {}", resource_);
}
Group::Group(Resource<> resource, KaufmanRobertsVariant kr_variant)
  : resource_(std::move(resource)), kr_variant_(kr_variant)
{
  debug_println("Group resource {}", resource_);
}

} // namespace Model
