
#include "pascal.h"

#include "logger.h"
#include "types.h"
#include "world.h"

PascalSourceStream::PascalSourceStream(const SourceName &name,
                                       Intensity intensity,
                                       Size sources_number,
                                       Size load_size)
  : SourceStream(name),
    intensity_(intensity),
    load_size_(load_size),
    sources_number_(sources_number)
{
}

void PascalSourceStream::notify_on_send(const LoadSendEvent *event)
{
  world_->schedule(produce_load(event->load.send_time));
}

void PascalSourceStream::notify_on_accept(const LoadSendEvent *event)
{
  active_sources_++;
  debug_print("{} Load has been accepted {}\n", *this, event->load);

  auto new_event = create_produce_load_event(event->load.send_time);
  linked_sources_.emplace(event->load.id, make_observer(new_event.get()));
  world_->schedule(std::move(new_event));
}

void PascalSourceStream::notify_on_serve(const LoadServeEvent *event)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, event->load);

  // TODO(PW): remove related event
  if (auto it = linked_sources_.find(event->load.id);
      it != end(linked_sources_)) {
    it->second->clear_type();
    linked_sources_.erase(it);
  }
}

void PascalSourceStream::notify_on_produce(const LoadProduceEvent *event)
{
  auto it =
      find_if(begin(linked_sources_), end(linked_sources_),
              [&event](const auto &p) { return p.second.get() == event; });
  if (it != linked_sources_.end()) {
    linked_sources_.erase(it);
  }
}
void PascalSourceStream::init()
{
  for (Size i = 0; i < sources_number_; ++i) {
    world_->schedule(produce_load(world_->get_time()));
  }
}

std::unique_ptr<LoadProduceEvent>
PascalSourceStream::create_produce_load_event(Time time)
{
  Duration dt{exponential(world_->get_random_engine())};
  return std::make_unique<LoadProduceEvent>(world_->get_uuid(), time + dt,
                                            this);
}

EventPtr PascalSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }

  Duration dt{exponential(world_->get_random_engine())};
  auto load = create_load(time + dt, load_size_);
  debug_print("{} Produced: {}\n", *this, load);

  return std::make_unique<LoadSendEvent>(world_->get_uuid(), load);
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const PascalSourceStream &source)
{
  f.writer().write(
      "[PascalSource {} (id={}), active={}/{}, gamma={}, lambda={}]",
      source.name_, source.id, source.active_sources_, source.sources_number_,
      source.intensity_,
      (source.sources_number_ - source.active_sources_) * source.intensity_);
}
