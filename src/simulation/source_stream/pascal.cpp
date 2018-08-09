
#include "pascal.h"

#include "logger.h"
#include "simulation/group.h"
#include "simulation/world.h"
#include "types.h"

#include <optional>

namespace Simulation
{
PascalSourceStream::PascalSourceStream(
    const SourceName &name, const TrafficClass &tc, Count sources_number)
  : SourceStream(name, tc), sources_number_(sources_number)
{
}

template <typename M>
std::optional<typename M::const_iterator>
find_event(const M &map, const Event *event)
{
  auto it = find_if(
      begin(map), end(map), [&event](const auto &p) { return p.second == event; });
  if (it != map.end()) {
    return std::make_optional(it);
  }
  return {};
}

void
PascalSourceStream::notify_on_skip_processing(const Event * /* event */)
{
}
void
PascalSourceStream::notify_on_request_service_start(const LoadServiceRequestEvent *event)
{
  auto new_event = create_produce_service_request(event->time);
  debug_print("{} on service start {}\n", *this, *new_event);

  if (auto it = find_event(linked_sources_, event); it.has_value()) {
    debug_print("{} INCEPTION!!! {}\n", *this, *event);
    linked_sources_count_++;
    linked_sources_.emplace(it.value()->first, new_event.get());
    debug_print(
        "{} [on service start] Add event {} linked to load id {}\n",
        *this,
        *new_event,
        it.value()->first);
  }
  world_->schedule(std::move(new_event));
}

void
PascalSourceStream::notify_on_request_drop(const LoadServiceRequestEvent *event)
{
  debug_print("{} Load has been dropped {}\n", *this, event->load);
  if (auto it = find_event(linked_sources_, event);
      it) { // the event is already linked to another request

    debug_print(
        "{} [on drop] Remove event {} linked to load id {}\n",
        *this,
        *it.value()->second,
        it.value()->first);
    linked_sources_.erase(it.value());
    linked_sources_count_--;
  }
}

void
PascalSourceStream::notify_on_request_accept(const LoadServiceRequestEvent *event)
{
  active_sources_++;
  debug_print("{} Load has been accepted {}\n", *this, event->load);
  if (auto it = find_event(linked_sources_, event); it.has_value()) {
    debug_print("{} INCEPTION!!! {}\n", *this, *event);
    debug_print(
        "{} [on drop] Remove event {} linked to load id {}\n",
        *this,
        *it.value()->second,
        it.value()->first);
    linked_sources_.erase(it.value());
  }

  // Create a new produce event linked to the currently served request
  auto new_event = create_produce_service_request(event->time);
  debug_print("{} on accept {}\n", *this, *new_event);

  linked_sources_count_++;
  linked_sources_.emplace(event->load.id, new_event.get());
  debug_print(
      "{} [on accept] Add event {} linked to load id {}\n",
      *this,
      *new_event,
      event->load.id);

  world_->schedule(std::move(new_event));
}

void
PascalSourceStream::notify_on_request_service_end(const LoadServiceEndEvent *event)
{
  active_sources_--;

  auto linked_event_range_it = linked_sources_.equal_range(event->load.id);
  // Remove scheduled new service request linked to the just ended service
  for (auto linked_event_it = linked_event_range_it.first;
       linked_event_it != linked_event_range_it.second;
       ++linked_event_it) {
    linked_sources_count_--;
    debug_print(
        "{} [on service end] Remove event {} linked to {}, load id {}\n",
        *this,
        *linked_event_it->second,
        *event,
        event->load.id);
    linked_event_it->second->skip_event();
  }
  linked_sources_.erase(linked_event_range_it.first, linked_event_range_it.second);
  debug_print("{} Load has been served {}\n", *this, event->load);
}

void
PascalSourceStream::notify_on_produce(const ProduceServiceRequestEvent *event)
{
  auto new_event = create_request(event->time);
  debug_print("{} on produce {}\n", *this, *new_event);

  if (auto it = find_event(linked_sources_, event);
      it.has_value()) { // the event is already linked to another request

    linked_sources_.emplace(it.value()->first, new_event.get());
    debug_print(
        "{} [on produce] Add event {} linked to load id {}\n",
        *this,
        *new_event,
        it.value()->first);

    it.value()->second->skip_event();
    linked_sources_.erase(it.value());
    debug_print(
        "{} [on produce] Remove event {} linked to load id {}\n",
        *this,
        *it.value()->second,
        it.value()->first);
  }

  world_->schedule(std::move(new_event));
}

void
PascalSourceStream::init()
{
  for (auto i = Count(0); i < sources_number_; ++i) {
    world_->schedule(create_produce_service_request(world_->get_time()));
  }
}

std::unique_ptr<ProduceServiceRequestEvent>
PascalSourceStream::create_produce_service_request(Time time)
{
  Duration dt{exponential(world_->get_random_engine())};
  return std::make_unique<ProduceServiceRequestEvent>(
      world_->get_uuid(), time + dt, this);
}

EventPtr
PascalSourceStream::create_request(Time time)
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

} // namespace Simulation
