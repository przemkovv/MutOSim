
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
  bool skip = false;

  Event(EventType type_, Uuid id_, Time time_);
  void clear_type();
  void skip_event();

  virtual void process();

  virtual ~Event() = default;
};

//----------------------------------------------------------------------

struct LoadServiceRequestEvent : public Event {
  Load load;

  LoadServiceRequestEvent(Uuid id, Load load_);

  void process() override;

  ~LoadServiceRequestEvent() override = default;
};

//----------------------------------------------------------------------

struct LoadServiceEndEvent : public Event {
  Load load;

  LoadServiceEndEvent(Uuid id, Load load_);

  void process() override;

  ~LoadServiceEndEvent() override = default;
};

//----------------------------------------------------------------------

struct ProduceServiceRequestEvent : public Event {
  std::experimental::observer_ptr<SourceStream> source_stream;

  ProduceServiceRequestEvent(Uuid id, Time time_, SourceStream *source_stream_);

  void process() override;
  ~ProduceServiceRequestEvent() override = default;
};

//----------------------------------------------------------------------

class by_time
{
public:
  bool operator()(const EventPtr &e1, const EventPtr &e2) const
  {
    return e1->time > e2->time;
  }
  bool operator()(const Event &e1, const Event &e2) const
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
