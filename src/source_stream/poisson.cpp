
#include "poisson.h"

#include "world.h"

PoissonSourceStream::PoissonSourceStream(const SourceName &name,
                                         Intensity intensity,
                                         Size load_size)
  : SourceStream(name), intensity_(intensity), load_size_(load_size)
{
}

void PoissonSourceStream::notify_on_send(const LoadSendEvent *event)
{
  world_->schedule(produce_load(event->load.send_time));
}

Size PoissonSourceStream::get_load_size()
{
  return load_size_;
}
Intensity PoissonSourceStream::get_intensity()
{
  return intensity_;
}
void PoissonSourceStream::init()
{
  world_->schedule(produce_load(world_->get_time()));
}

EventPtr PoissonSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_->get_uuid(), time);
  }
  Duration dt{exponential(world_->get_random_engine())};
  auto load = create_load(time + dt, load_size_);
  debug_print("{} Produced: {}\n", *this, load);

  return std::make_unique<LoadSendEvent>(world_->get_uuid(), load);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const PoissonSourceStream &source)
{
  f.writer().write("[PoissonSource {} (id={})]", source.name_, source.id);
}
