
#pragma once

#include "load.h"
#include "types.h"

#include <experimental/memory>
#include <fmt/format.h>
#include <gsl/gsl>
#include <memory>

struct Event;
class World;
using EventFunc = void (*)(World *, Event *);
using EventPtr = std::unique_ptr<Event>;

enum class EventType { LoadSend, LoadServe, LoadProduce, None };

struct Event {
  EventType type;
  Uuid id;
  Time time;

  Event(EventType type_, Uuid id_, Time time_)
    : type(type_), id(id_), time(time_)
  {
  }
  void clear_type() { type = EventType::None; }

  virtual void process();

  virtual ~Event();
};

struct LoadSendEvent : public Event {
  Load load;

  LoadSendEvent(Uuid id, Load load_)
    : Event(EventType::LoadSend, id, load_.send_time), load(std::move(load_))
  {
  }

  void process() override;

  ~LoadSendEvent() override;
};

struct LoadServeEvent : public Event {
  Load load;

  LoadServeEvent(Uuid id, Load load_)
    : Event(EventType::LoadServe, id, load_.end_time), load(std::move(load_))
  {
  }

  void process() override;

  ~LoadServeEvent() override;
};

struct LoadProduceEvent : public Event {
  std::experimental::observer_ptr<SourceStream> source_stream;

  LoadProduceEvent(Uuid id, Time time_, SourceStream *source_stream_)
    : Event(EventType::LoadProduce, id, time_), source_stream(source_stream_)
  {
  }

  void process() override;
  ~LoadProduceEvent() override;
};

class by_time
{
public:
  bool operator()(const EventPtr &e1, const EventPtr &e2)
  {
    return e1->time > e2->time;
  }
  bool operator()(const Event &e1, const Event &e2)
  {
    return e1.time > e2.time;
  }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Event &event);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EventType &type);
