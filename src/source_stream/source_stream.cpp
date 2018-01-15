
#include "source_stream.h"

#include "world.h"

SourceStream::SourceStream(const SourceName &name, const TrafficClass &tc)
  : name_(name), tc_(tc)
{
}
Load SourceStream::create_load(Time time, Size size)
{
  loads_produced_++;
  return {.id = LoadId{world_->get_uuid()},
          .tc_id = tc_.id,
          .send_time = time,
          .size = size,
          .produced_by = make_observer(this),
          .target_group = target_group_};
}

void SourceStream::notify_on_produce(const ProduceServiceRequestEvent * /* event */)
{
}
void SourceStream::notify_on_service_start(const LoadServiceRequestEvent * /* event */)
{
}
void SourceStream::notify_on_service_end(const LoadServiceEndEvent * /* event */)
{
}

void SourceStream::notify_on_service_accept(const LoadServiceRequestEvent * /* event */)
{
}
void SourceStream::notify_on_service_drop(const LoadServiceRequestEvent * /* event */)
{
}
void SourceStream::notify_on_skip_processing(const Event * /* event */)
{
}
void SourceStream::init()
{
}

void SourceStream::attach_to_group(gsl::not_null<Group *> target_group)
{
  target_group_ = make_observer(target_group.get());
}

void SourceStream::set_world(gsl::not_null<World *> world)
{
  world_ = make_observer(world.get());
}

void SourceStream::print_stats()
{
  print("{}: {} produced loads: {:n}\n", *this, tc_, loads_produced_);
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const SourceStream &source)
{
  f.writer().write("[Source {} id={:>2}]", source.name_, source.id);
  // f.writer().write("[Source {} (id={:>2})]", source.world_->get_current_time(),
  // source.name_, source.id);
}
