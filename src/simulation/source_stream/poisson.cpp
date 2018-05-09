
#include "poisson.h"

#include "simulation/group.h"
#include "simulation/world.h"

#include <fmt/ostream.h>

namespace Simulation
{
PoissonSourceStream::PoissonSourceStream(const SourceName &name, const TrafficClass &tc)
  : SourceStream(name, tc)
{
}

void
PoissonSourceStream::notify_on_service_start(const LoadServiceRequestEvent *event)
{
  world_->schedule(produce_load(event->load.send_time));
}

void
PoissonSourceStream::init()
{
  world_->schedule(produce_load(world_->get_time()));
}

EventPtr
PoissonSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }
  Duration dt{exponential(world_->get_random_engine())};
  auto load = create_load(time + dt, tc_.size);
  debug_print("{} Produced: {}\n", *this, load);

  return std::make_unique<LoadServiceRequestEvent>(world_->get_uuid(), load);
}

void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const PoissonSourceStream &source)
{
  f.writer().write("[PoissonSource {} (id={})]", source.name_, source.id);
}
} // namespace Simulation
