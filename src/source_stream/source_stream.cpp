
#include "source_stream.h"

#include "world.h"

Load SourceStream::create_load(Time time, Size size)
{
  return {
      LoadId{world_->get_uuid()}, tc_.id,       time, size, Time{-1}, false, {},
      make_observer(this),        target_group_};
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
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const SourceStream &source)
{
  f.writer().write("[Source {} (id={}, t={}, int={})]", source.name_, source.id,
                   source.get_load_size(), source.get_intensity());
}
