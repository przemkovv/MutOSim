#include "event.h"

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
