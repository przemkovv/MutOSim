#include "engset.h"

#include "simulation/group.h"
#include <fmt/ostream.h>

EngsetSourceStream::EngsetSourceStream(const SourceName &name,
                                       const TrafficClass &tc,
                                       Count sources_number)
  : SourceStream(name, tc), sources_number_(sources_number)
{
}

void EngsetSourceStream::notify_on_produce(const ProduceServiceRequestEvent *event)
{
  world_->schedule(produce_load(event->time));
}

void EngsetSourceStream::notify_on_service_end(const LoadServiceEndEvent *event)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, event->load);

  if (active_sources_ < Count(0)) {
    print("{} Number of active sources is less than zero. Load {}\n", *this,
          event->load);
    std::abort();
  }

  world_->schedule(create_produce_service_request(event->load.end_time));
}

void EngsetSourceStream::notify_on_service_accept(const LoadServiceRequestEvent *event) {
    debug_print("{} Service accepted: {}\n", *this, event->load);
    active_sources_++;
}

void EngsetSourceStream::notify_on_service_drop(const LoadServiceRequestEvent *event) {
    debug_print("{} Service dropped: {}\n", *this, event->load);
    world_->schedule(create_produce_service_request(event->time));
}

void EngsetSourceStream::init()
{
  for (auto i = Count(0); i < sources_number_; ++i) {
    world_->schedule(create_produce_service_request(world_->get_time()));
  }
}

std::unique_ptr<ProduceServiceRequestEvent>
EngsetSourceStream::create_produce_service_request(Time time)
{
  Duration dt{exponential(world_->get_random_engine())};
  return std::make_unique<ProduceServiceRequestEvent>(world_->get_uuid(), time + dt,
                                            this);
}

EventPtr EngsetSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }

  auto load = create_load(time, tc_.size);
  return std::make_unique<LoadServiceRequestEvent>(world_->get_uuid(), load);
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EngsetSourceStream &source)
{
  f.writer().write(
      "[EngsetSource {} (id={}), active={}/{}, gamma={}, lambda={}]",
      source.name_, source.id, source.active_sources_, source.sources_number_,
      source.tc_.serve_intensity,
      (source.sources_number_ - source.active_sources_) * source.tc_.serve_intensity);
}
