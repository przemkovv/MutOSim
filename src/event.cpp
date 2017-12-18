#include "event.h"

#include "group.h"
#include "source_stream/source_stream.h"

Event::~Event()
{
}

LoadServeEvent::~LoadServeEvent()
{
}
LoadSendEvent::~LoadSendEvent()
{
}
LoadProduceEvent::~LoadProduceEvent()
{
}
void Event::process()
{
}
void LoadProduceEvent::process()
{
  source_stream->notify_on_produce(this);
}
void LoadServeEvent::process()
{
  load.produced_by->notify_on_serve(this);
  load.served_by->notify_on_serve(this);
}

void LoadSendEvent::process()
{
  load.produced_by->notify_on_send(this);
}
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
