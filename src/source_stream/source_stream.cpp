
#include "source_stream.h"

#include "world.h"

EventPtr SourceStream::produce_load(Time time)
{
  Load load{world_->get_uuid(), time, 1, false, {}, target_group_};
  return std::make_unique<LoadSendEvent>(world_->get_uuid(), load);
}

void SourceStream::notify_on_produce(const LoadProduceEvent * /* event */)
{
}
void SourceStream::notify_on_send(const LoadSendEvent * /* event */)
{
}
void SourceStream::notify_on_serve(const LoadServeEvent * /* event */)
{
}

void SourceStream::notify_on_accept(const LoadSendEvent * /* event */)
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
  id = world->get_uuid();
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const SourceStream &source)
{
  f.writer().write("[Source {} (id={})]", source.name_, source.id);
}
