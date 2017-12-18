
#pragma once

#include "load.h"
#include "types.h"

#include <experimental/memory>
#include <fmt/format.h>
#include <gsl/gsl>
#include <memory>

struct Event;
class World;
using EventPtr = std::unique_ptr<Event>;

enum class EventType { LoadSend, LoadServe, LoadProduce, None };

//----------------------------------------------------------------------

struct Event {
  EventType type;
  Uuid id;
  Time time;

  Event(EventType type_, Uuid id_, Time time_);
  void clear_type();

  virtual void process();

  virtual ~Event() = default;
};

//----------------------------------------------------------------------

struct LoadSendEvent : public Event {
  Load load;

  LoadSendEvent(Uuid id, Load load_);

  void process() override;

  ~LoadSendEvent() override = default;
};

//----------------------------------------------------------------------

struct LoadServeEvent : public Event {
  Load load;

  LoadServeEvent(Uuid id, Load load_);

  void process() override;

  ~LoadServeEvent() override = default;
};

//----------------------------------------------------------------------

struct LoadProduceEvent : public Event {
  std::experimental::observer_ptr<SourceStream> source_stream;

  LoadProduceEvent(Uuid id, Time time_, SourceStream *source_stream_);

  void process() override;
  ~LoadProduceEvent() override = default;
};

//----------------------------------------------------------------------

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

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Event &event);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EventType &type);
