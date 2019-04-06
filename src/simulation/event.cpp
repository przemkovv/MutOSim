#include "event.h"

#include "group.h"
#include "source_stream/source_stream.h"

namespace Simulation {
//----------------------------------------------------------------------

Event::Event(EventType type_, Uuid id_, Time time_) : type(type_), id(id_), time(time_)
{
}

void
Event::clear_type()
{
  type = EventType::None;
}

void
Event::skip_event()
{
  skip = true;
}

void
Event::skip_notify()
{
}

void
Event::process()
{
}
//----------------------------------------------------------------------

LoadServiceRequestEvent::LoadServiceRequestEvent(Uuid id_, Load load_)
  : Event(EventType::LoadServiceRequest, id_, load_.send_time), load(load_)
{
}

void
LoadServiceRequestEvent::process()
{
  load.produced_by->notify_on_request_service_start(this);

  if (load.target_group->try_serve(load))
  {
    load.produced_by->notify_on_request_accept(this);
  }
  else
  {
    load.produced_by->notify_on_request_drop(this);
  }
}
void
LoadServiceRequestEvent::skip_notify()
{
  load.produced_by->notify_on_skip_processing(static_cast<Event *>(this));
}

//----------------------------------------------------------------------

ProduceServiceRequestEvent::ProduceServiceRequestEvent(
    Uuid          id_,
    Time          time_,
    SourceStream *source_stream_)
  : Event(EventType::LoadProduce, id_, time_), source_stream(source_stream_)
{
}

void
ProduceServiceRequestEvent::process()
{
  source_stream->notify_on_produce(this);
}
void
ProduceServiceRequestEvent::skip_notify()
{
  source_stream->notify_on_skip_processing(static_cast<Event *>(this));
}

//----------------------------------------------------------------------

LoadServiceEndEvent::LoadServiceEndEvent(Uuid id_, Load load_)
  : Event(EventType::LoadServiceEnd, id_, load_.end_time), load((load_))
{
}

void
LoadServiceEndEvent::process()
{
  load.served_by.back()->notify_on_request_service_end(this);
  load.produced_by->notify_on_request_service_end(this);
}
void
LoadServiceEndEvent::skip_notify()
{
  load.produced_by->notify_on_skip_processing(static_cast<Event *>(this));
}

//----------------------------------------------------------------------

} // namespace Simulation
