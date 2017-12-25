#include "engset.h"

#include "group.h"

EngsetSourceStream::EngsetSourceStream(const SourceName &name,
                                       Intensity intensity,
                                       Count sources_number,
                                       Size load_size)
  : SourceStream(name),
    intensity_(intensity),
    load_size_(load_size),
    sources_number_(sources_number)
{
}

void EngsetSourceStream::notify_on_produce(const LoadProduceEvent *event)
{
  world_->schedule(produce_load(event->time));
}

void EngsetSourceStream::notify_on_serve(const LoadServeEvent *event)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, event->load);

  if (active_sources_ < Count(0)) {
    print("{} Number of active sources is less than zero. Load {}\n", *this,
          event->load);
    std::abort();
  }

  world_->schedule(create_produce_load_event(event->load.end_time));
}

Size EngsetSourceStream::get_load_size() const
{
  return load_size_;
}
Intensity EngsetSourceStream::get_intensity() const
{
  return intensity_;
}
void EngsetSourceStream::init()
{
  for (auto i = Count(0); i < sources_number_; ++i) {
    world_->schedule(create_produce_load_event(world_->get_time()));
  }
}

std::unique_ptr<LoadProduceEvent>
EngsetSourceStream::create_produce_load_event(Time time)
{
  Duration dt{exponential(world_->get_random_engine())};
  return std::make_unique<LoadProduceEvent>(world_->get_uuid(), time + dt,
                                            this);
}

EventPtr EngsetSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }

  auto load = create_load(time, load_size_);
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

//----------------------------------------------------------------------

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
