#include "engset.h"

#include "group.h"

EngsetSourceStream::EngsetSourceStream(const Name &name,
                                       Intensity intensity,
                                       Size sources_number,
                                       Size load_size)
  : SourceStream(name),
    intensity_(intensity),
    load_size_(load_size),
    sources_number_(sources_number)
{
}

void engset_load_produce_callback(World *world, Event *event)
{
  auto produce_event = static_cast<LoadProduceEvent *>(event);
  auto &source_stream = produce_event->source_stream;
  world->schedule(source_stream->produce_load(event->time));
}

void EngsetSourceStream::notify_on_serve(const Load &load)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, load);

  if (active_sources_ < 0) {
    print("{} Number of active sources is less than zero. Load {}\n", *this,
          load);
    std::abort();
  }

  world_->schedule(create_produce_load_event(load.end_time));
}

void EngsetSourceStream::init()
{
  for (Size i = 0; i < sources_number_; ++i) {
    world_->schedule(create_produce_load_event(world_->get_time()));
  }
}
Load EngsetSourceStream::create_load(Time time)
{
  return {world_->get_uuid(),  time,         load_size_, -1, false, {},
          make_observer(this), target_group_};
}

std::unique_ptr<LoadProduceEvent>
EngsetSourceStream::create_produce_load_event(Time time)
{
  // auto params = decltype(exponential)::param_type(
  // (sources_number_ - active_sources_) * intensity_);
  // exponential.param(params);
  auto dt = static_cast<Time>(exponential(world_->get_random_engine()));
  return std::make_unique<LoadProduceEvent>(world_->get_uuid(), time + dt, this,
                                            engset_load_produce_callback);
}

EventPtr EngsetSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }
  if (!target_group_) {
    print("{} The source is not connected to any group.", *this);
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }
  auto load = create_load(time);
  if (target_group_->can_serve(load.size)) {
    debug_print("{} Produced: {}\n", *this, load);
    active_sources_++;
  } else {
    debug_print("{} Produced ghost: {}\n", *this, load);
    load.drop = true;
    world_->schedule(create_produce_load_event(time));
  }
  return std::make_unique<LoadSendEvent>(world_->get_uuid(), load);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EngsetSourceStream &source)
{
  f.writer().write(
      "[EngsetSource {} (id={}), active={}/{}, gamma={}, lambda={}]",
      source.name_, source.id, source.active_sources_, source.sources_number_,
      source.intensity_,
      (source.sources_number_ - source.active_sources_) * source.intensity_);
}
