#pragma once

#include "types.h"

enum class EventType { Arrival, Serve };

struct Event {
  Time time;
  EventType event_type;

};

struct ArrivalEvent : public Event {

};

struct ServeEvent : public Event {
};

bool operator<(const Event &e1, const Event &e2);
