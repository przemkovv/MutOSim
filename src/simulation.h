#pragma once

#include "types.h"

#include <queue>

struct Simulation {
  Time time = 0;
  TimePeriod length = 10000;
  TimePeriod tick_length = 3;

  std::priority_queue<Event> events{};
  std::queue<Request> requests{};

  Time advance()
  {
    time += tick_length;
    return time;
  }

  void queue_event(Time t, EventType type) {
    events.emplace(Event{t, type});
  }
};
