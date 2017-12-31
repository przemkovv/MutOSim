#include "event.h"

#include "group.h"
#include "source_stream/source_stream.h"

//----------------------------------------------------------------------


Event::Event(EventType type_, Uuid id_, Time time_)
  : type(type_), id(id_), time(time_)
{
}

void Event::clear_type()
{
  type = EventType::None;
}

void Event::skip_event()
{
  skip = true;
}

void Event::process()
{
}
//----------------------------------------------------------------------

LoadServiceRequestEvent::LoadServiceRequestEvent(Uuid id, Load load_)
  : Event(EventType::LoadSend, id, load_.send_time), load(load_)
{
}

void LoadServiceRequestEvent::process()
{
  load.produced_by->notify_on_service_start(this);

  if (load.target_group->try_serve(load)) {
    load.produced_by->notify_on_service_accept(this);
  } else {
    load.produced_by->notify_on_service_drop(this);
  }
}

//----------------------------------------------------------------------

ProduceServiceRequestEvent::ProduceServiceRequestEvent(Uuid id,
                                   Time time_,
                                   SourceStream *source_stream_)
  : Event(EventType::LoadProduce, id, time_), source_stream(source_stream_)
{
}

void ProduceServiceRequestEvent::process()
{
  source_stream->notify_on_produce(this);
}

//----------------------------------------------------------------------

LoadServiceEndEvent::LoadServiceEndEvent(Uuid id, Load load_)
  : Event(EventType::LoadServe, id, load_.end_time), load((load_))
{
}

void LoadServiceEndEvent::process()
{
  load.served_by->notify_on_service_end(this);
  load.produced_by->notify_on_service_end(this);
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EventType &type)
{
  f.writer().write([type]() {
    switch (type) {
    case EventType::LoadSend:
      return "LoadSend";
    case EventType::LoadServe:
      return "LoadServe";
    case EventType::LoadProduce:
      return "LoadProduce";
    case EventType::None:
      return "None";
    }
  }());
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Event &event)
{
  f.writer().write("[Event: id={}, type={}, t={}]", event.id, event.type,
                   event.time);
}
