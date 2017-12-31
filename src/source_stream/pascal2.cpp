
#include "pascal2.h"

#include "logger.h"
#include "types.h"
#include "world.h"

#include <optional>

Pascal2SourceStream::Pascal2SourceStream(const SourceName &name,
                                         const TrafficClass &tc,
                                         Count sources_number)
  : SourceStream(name, tc), sources_number_(sources_number)
{
}

template <typename M>
std::optional<typename M::const_iterator> find_event(const M &map, const Event *event)
{
  auto it = find_if(begin(map), end(map),
                    [&event](const auto &p) { return p.second.get() == event; });
  if (it != map.end()) {
    return std::make_optional(it);
  } else {
    return {};
  }
}

void Pascal2SourceStream::notify_on_service_start(const LoadServiceRequestEvent *event)
{
  auto new_event = create_produce_service_request(event->time);
  debug_print("{} on send {}\n", *this, *static_cast<Event *>(new_event.get()));
  world_->schedule(std::move(new_event));
}

void Pascal2SourceStream::notify_on_service_drop(const LoadServiceRequestEvent *event)
{
  debug_print("{} Load has been dropped {}\n", *this, event->load);
}

void Pascal2SourceStream::notify_on_service_accept(const LoadServiceRequestEvent *event)
{
  active_sources_++;
  debug_print("{} Load has been accepted {}\n", *this, event->load);

  // Create a new produce event linked to the currently served request
  auto new_event = create_produce_service_request(event->load.send_time);
  debug_print("{} on accept {}\n", *this, *static_cast<Event *>(new_event.get()));

  linked_sources_.emplace(event->load.id, make_observer(new_event.get()));
  linked_sources_count_++;

  world_->schedule(std::move(new_event));
}

void Pascal2SourceStream::notify_on_service_end(const LoadServiceEndEvent *event)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, event->load);

  //Remove scheduled new service request linked to the just ended service
  if (auto linked_event_it = linked_sources_.find(event->load.id);
      linked_event_it != linked_sources_.end()) {
    linked_event_it->second->skip_event();
    linked_sources_.erase(linked_event_it);
  }
}

void Pascal2SourceStream::notify_on_produce(const ProduceServiceRequestEvent *event)
{
  LoadId original_load_id{0};
  auto new_event = produce_load(event->time);

  debug_print("{} on produce {}\n", *this, *static_cast<Event *>(new_event.get()));
  world_->schedule(std::move(new_event));
}

void Pascal2SourceStream::init()
{
  for (auto i = Count(0); i < sources_number_; ++i) {
    world_->schedule(create_produce_service_request(world_->get_time()));
  }
}

std::unique_ptr<ProduceServiceRequestEvent>
Pascal2SourceStream::create_produce_service_request(Time time)
{
  Duration dt{exponential(world_->get_random_engine())};
  return std::make_unique<ProduceServiceRequestEvent>(world_->get_uuid(), time + dt,
                                                      this);
}

EventPtr Pascal2SourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }

  // Duration dt{exponential(world_->get_random_engine())};
  auto load = create_load(time, tc_.size);
  debug_print("{} Produced: {}\n", *this, load);

  return std::make_unique<LoadServiceRequestEvent>(world_->get_uuid(), load);
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Pascal2SourceStream &source)
{
  f.writer().write("t={} [Pascal2Source {} (id={}), active={}/{}/{}, gamma={}]",
                   source.world_->get_current_time(), source.name_, source.id,
                   source.active_sources_, source.sources_number_,
                   source.linked_sources_count_, source.tc_.source_intensity);
}
