
#pragma once

#include "load.h"
#include "types/types.h"

#include <memory>

namespace Simulation {
struct Event;
using EventPtr = std::unique_ptr<Event>;

enum class EventType { LoadServiceRequest, LoadServiceEnd, LoadProduce, None };

//----------------------------------------------------------------------

struct Event
{
  EventType type;
  Uuid      id;
  Time      time;
  bool      skip = false;

  Event(EventType type_, Uuid id_, Time time_);
  void clear_type();
  void skip_event();

  virtual void process();
  virtual void skip_notify();

  virtual ~Event() = default;
};

//----------------------------------------------------------------------

struct LoadServiceRequestEvent : public Event
{
  Load load;

  LoadServiceRequestEvent(Uuid id, Load load_);

  void process() override;
  void skip_notify() override;
};

//----------------------------------------------------------------------

struct LoadServiceEndEvent : public Event
{
  Load load;

  LoadServiceEndEvent(Uuid id, Load load_);

  void process() override;
  void skip_notify() override;
};

//----------------------------------------------------------------------

struct ProduceServiceRequestEvent : public Event
{
  SourceStream *source_stream;

  ProduceServiceRequestEvent(Uuid id, Time time_, SourceStream *source_stream_);
  ProduceServiceRequestEvent(const ProduceServiceRequestEvent &) = delete;
  ProduceServiceRequestEvent &operator=(const ProduceServiceRequestEvent &) = delete;

  void process() override;
  void skip_notify() override;
};

//----------------------------------------------------------------------

class by_time
{
public:
  bool operator()(const EventPtr &e1, const EventPtr &e2) const
  {
    if (e1->time != e2->time)
    {
      return e1->time > e2->time;
    }
    return e1->id < e1->id;
  }
  bool operator()(const Event &e1, const Event &e2) const
  {
    if (e1.time != e2.time)
    {
      return e1.time > e2.time;
    }
    return e1.id < e2.id;
  }
};

//----------------------------------------------------------------------

} // namespace Simulation
