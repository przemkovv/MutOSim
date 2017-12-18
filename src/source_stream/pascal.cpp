
#include "pascal.h"

#include "logger.h"
#include "types.h"
#include "world.h"

PascalSourceStream::PascalSourceStream(const Name &name,
                                       Intensity intensity,
                                       Size sources_number,
                                       Size load_size)
  : SourceStream(name),
    intensity_(intensity),
    load_size_(load_size),
    sources_number_(sources_number)
{
}

static void pascal_produce_load_callback(World *world, Event *event)
{
  if (event->type == EventType::LoadSend) {
    auto send_event = static_cast<LoadSendEvent *>(event);
    auto &produced_by = send_event->load.produced_by;
    world->schedule(produced_by->produce_load(send_event->load.send_time));
  }
}
void PascalSourceStream::notify_on_accept(const Load &load)
{
  active_sources_++;
  debug_print("{} Load has been accepted {}\n", *this, load);

  auto event = create_produce_load_event(load.send_time);
  linked_sources_.emplace(load.id, make_observer(event.get()));
  world_->schedule(std::move(event));
}

void PascalSourceStream::notify_on_serve(const Load &load)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, load);

  // TODO(PW): remove related event
  if (auto it = linked_sources_.find(load.id); it != end(linked_sources_)) {
    it->second->clear_type();
    linked_sources_.erase(it);
  }
}

void PascalSourceStream::notify_on_produce(
    const LoadProduceEvent *produce_event)
{
  auto it = find_if(begin(linked_sources_), end(linked_sources_),
                    [&produce_event](const auto &p) {
                      return p.second.get() == produce_event;
                    });
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
Load PascalSourceStream::create_load(Time time)
{
  return {world_->get_uuid(),  time,         load_size_, -1, false, {},
          make_observer(this), target_group_};
}

std::unique_ptr<LoadProduceEvent>
PascalSourceStream::create_produce_load_event(Time time)
{
  // auto params = decltype(exponential)::param_type(
  // (sources_number_ - active_sources_) * intensity_);
  // exponential.param(params);
  auto dt = static_cast<Time>(exponential(world_->get_random_engine()));
  return std::make_unique<LoadProduceEvent>(world_->get_uuid(), time + dt, this,
                                            pascal_produce_load_callback);
}

EventPtr PascalSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }

  auto dt = static_cast<Time>(exponential(world_->get_random_engine()));
  auto load = create_load(time + dt);
  debug_print("{} Produced: {}\n", *this, load);

  return std::make_unique<LoadSendEvent>(world_->get_uuid(), load,
                                         pascal_produce_load_callback);
}

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
