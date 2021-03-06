
#include "source_stream.h"

#include "simulation/group.h"
#include "simulation/world.h"
#include "source_stream_format.h"
#include "types/types_format.h"

namespace Simulation {
SourceStream::SourceStream(const SourceName &name, const TrafficClass &tc) : name_(name), tc_(tc)
{
}
Load
SourceStream::create_load(Time time, Size size)
{
  loads_produced_++;

  Load load;
  load.id = LoadId{world_->get_uuid()};
  load.tc_id = tc_.id;
  load.send_time = time;
  load.size = size;
  load.produced_by = this;
  load.target_group = target_group_;
  return load;
}

void
SourceStream::notify_on_produce(const ProduceServiceRequestEvent * /* event */)
{
}
void
SourceStream::notify_on_request_service_start(const LoadServiceRequestEvent * /* event */)
{
}
void
SourceStream::notify_on_request_service_end(const LoadServiceEndEvent * /* event */)
{
}

void
SourceStream::notify_on_request_accept(const LoadServiceRequestEvent * /* event */)
{
}
void
SourceStream::notify_on_request_drop(const LoadServiceRequestEvent * /* event */)
{
}
void
SourceStream::notify_on_skip_processing(const Event * /* event */)
{
}
void
SourceStream::init()
{
}

void
SourceStream::attach_to_group(Group &target_group)
{
  target_group_ = &target_group;
}

void
SourceStream::set_world(World &world)
{
  world_ = &world;
}

void
SourceStream::print_stats()
{
  print("{}: {} produced loads: {:n}\n", *this, tc_, loads_produced_);
}

} // namespace Simulation
