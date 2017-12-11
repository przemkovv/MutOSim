
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <random>
#include <vector>

using Time = int64_t;
using TimePeriod = int64_t;
using RequestId = uint64_t;

template <typename T> RequestId to_id(T value)
{
  return static_cast<RequestId>(value);
}

namespace Math {

int64_t factorial(int64_t k)
{
  int64_t result = 1;
  while (k-- != 0) {
    result *= k;
  }
  return result;
}
}; // namespace Math

struct Request {
  RequestId id;
  int64_t count;
};

struct Event {
  Time time;
};

bool operator<(const Event &e1, const Event &e2)
{
  return e1.time < e2.time;
}

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
};

struct ArrivalEvent : public Event {
};

struct ServeEvent : public Event {
};

class RequestStream {
public:
  virtual Request get(Time time)
  {
    Request r{to_id(time), 1};
    return r;
  }

  RequestStream() = default;
  RequestStream(RequestStream &&) = default;
  RequestStream(const RequestStream &) = default;
  RequestStream &operator=(const RequestStream &) = default;
  RequestStream &operator=(RequestStream &&) = default;
  virtual ~RequestStream() = default;
};

template <typename RandomEngine>
class PoissonRequestStream : public RequestStream {

  RandomEngine random_engine;

  float intensity;
  TimePeriod time_period;

  std::poisson_distribution<int64_t> d;

public:
  PoissonRequestStream(RandomEngine random_engine,
                       float intensity,
                       TimePeriod time_period)
    : random_engine(random_engine),
      intensity(intensity),
      time_period(time_period),
      d(intensity * time_period)
  {
  }

  float Pk(const int k, const Time t)
  {
    return std::pow(intensity * t, k) / Math::factorial(k) *
           expf(-intensity * t);
  }

  Request get(Time t) override { return {to_id(t), d(random_engine)}; }
};

int main()
{
  std::random_device rd;

  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<float> dis;

  Simulation sim{0, 10000, 3};
  PoissonRequestStream request_stream{gen, 0.25, sim.tick_length};

  int64_t events_number = 0;

  std::map<int64_t, int64_t> events_counts;

  while (sim.advance() < sim.length) {

    auto request = request_stream.get(1);

    events_number += request.count;

    ++events_counts[request.count];
  }

  fmt::print("\n{}/{}= {}\n", events_number, sim.length,
             static_cast<float>(events_number) / sim.length);

  for (auto p : events_counts) {
    std::cout << p.first << ' ' << std::string(p.second / 100, '*') << '\n';
  }

  return 0;
}
