
#include "pascal.h"

#include "logger.h"
#include "types.h"
#include "world.h"

#include <optional>

PascalSourceStream::PascalSourceStream(const SourceName &name,
                                       const TrafficClass &tc,
                                       Count sources_number)
  : SourceStream(name, tc), sources_number_(sources_number)
{
}

template <typename M>
std::optional<typename M::const_iterator> find_event(const M &map,
                                                     const Event *event)
{
  auto it = find_if(begin(map), end(map), [&event](const auto &p) {
    return p.second.get() == event;
  });
  if (it != map.end()) {
    return std::make_optional(it);
  } else {
    return {};
  }
}

void PascalSourceStream::notify_on_service_start(const LoadServiceRequestEvent *event)
{
  if (auto result = find_event(closing_linked_sources_, event); result) {
    auto &it = result.value();
    debug_print(
        "{} Closing pending linked source linked to LoadId {} and event {}\n",
        *this, it->first, *static_cast<const Event *>(it->second.get()));
    closing_linked_sources_.erase(it);
    linked_sources_count_--;
  } else {
    auto new_event = create_produce_service_request(event->time);
    debug_print("{} on send {}\n", *this,
                *static_cast<Event *>(new_event.get()));
    world_->schedule(std::move(new_event));
  }
}

void PascalSourceStream::notify_on_service_drop(const LoadServiceRequestEvent *event)
{
  debug_print("{} Load has been dropped {}\n", *this, event->load);

  if (auto result = find_event(linked_sources_, event); result) {
    auto &it = result.value();
    debug_print("{} D Closing source linked to LoadId {} and event {}\n", *this,
                it->first, *static_cast<const Event *>(it->second.get()));
    linked_sources_.erase(it);
    linked_sources_count_--;
  } else {
  }
}

void PascalSourceStream::notify_on_service_accept(const LoadServiceRequestEvent *event)
{
  active_sources_++;
  debug_print("{} Load has been accepted {}\n", *this, event->load);

  auto new_event = create_produce_service_request(event->load.send_time);
  debug_print("{} on accept {}\n", *this,
              *static_cast<Event *>(new_event.get()));

  debug_print("{} New event {} linked to {}\n", *this,
              *static_cast<const Event *>(new_event.get()),
              *static_cast<const Event *>(event));
  linked_sources_.emplace(event->load.id, make_observer(new_event.get()));
  linked_sources_count_++;

  world_->schedule(std::move(new_event));
}

void PascalSourceStream::notify_on_service_end(const LoadServiceEndEvent *event)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, event->load);

  if (auto it = linked_sources_.find(event->load.id);
      it != end(linked_sources_)) {
    // if (it->second->type == EventType::LoadServiceRequest) {
      // // Load using linked source was already send
      // closing_linked_sources_.insert(*it);
      // debug_print(
          // "{} Waiting for closing source linked to LoadId {} and event {}\n",
          // *this, it->first, *static_cast<const Event *>(it->second.get()));
    // } else {
      it->second->skip_event();
      linked_sources_count_--;
      debug_print("{} Closing source linked to LoadId {} and event {}\n", *this,
                  it->first, *static_cast<const Event *>(it->second.get()));
    // }
    linked_sources_.erase(it);
  }
}

void PascalSourceStream::notify_on_produce(const ProduceServiceRequestEvent *event)
{
  LoadId original_load_id{0};
  auto new_event = produce_load(event->time);

  if (auto result = find_event(linked_sources_, event); result) {
    auto &it = result.value();

    // Current produce event is linked to another load
    original_load_id = it->first;
    linked_sources_.erase(it);
    linked_sources_.emplace(
        original_load_id, make_observer(static_cast<Event *>(new_event.get())));
    debug_print("{} on produce {}, linked to load id {}\n", *this,
                *static_cast<Event *>(new_event.get()), original_load_id);
  } else {
    debug_print("{} on produce {}\n", *this,
                *static_cast<Event *>(new_event.get()));
  }
  world_->schedule(std::move(new_event));
}

void PascalSourceStream::init()
{
  for (auto i = Count(0); i < sources_number_; ++i) {
    world_->schedule(create_produce_service_request(world_->get_time()));
  }
}

std::unique_ptr<ProduceServiceRequestEvent>
PascalSourceStream::create_produce_service_request(Time time)
{
  Duration dt{exponential(world_->get_random_engine())};
  return std::make_unique<ProduceServiceRequestEvent>(world_->get_uuid(), time + dt,
                                            this);
}

EventPtr PascalSourceStream::produce_load(Time time)
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
                const PascalSourceStream &source)
{
  f.writer().write("[PascalSource {} (id={}), active={}/{}/{}, gamma={}]",
                   source.name_, source.id, source.active_sources_,
                   source.sources_number_, source.linked_sources_count_,
                   source.tc_.source_intensity);
}
