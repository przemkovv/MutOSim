
#include "poisson.h"

#include "simulation/group.h"
#include "simulation/load_format.h"
#include "simulation/source_stream/source_stream_format.h"
#include "simulation/world.h"
#include "types/types_format.h"

namespace Simulation
{
PoissonSourceStream::PoissonSourceStream(const SourceName &name, const TrafficClass &tc)
  : SourceStream(name, tc)
{
}

void
PoissonSourceStream::notify_on_request_service_start(const LoadServiceRequestEvent *event)
{
  world_->schedule(create_request(event->load.send_time));
}

void
PoissonSourceStream::init()
{
  world_->schedule(create_request(world_->get_time()));
}

EventPtr
PoissonSourceStream::create_request(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }
  Duration dt{exponential(world_->get_random_engine())};
  auto load = create_load(time + dt, tc_.size);
  debug_print("{} Produced: {}\n", *this, load);

  return std::make_unique<LoadServiceRequestEvent>(world_->get_uuid(), load);
}

} // namespace Simulation
