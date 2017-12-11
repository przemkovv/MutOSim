
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <optional>
#include <random>

using Time = int64_t;
using TimePeriod = int64_t;
using RequestId = uint64_t;

static std::random_device rd;

static std::mt19937_64 gen(0);
static std::uniform_real_distribution<float> dis;

template <typename T> RequestId to_id(T value)
{
  return static_cast<RequestId>(value);
}

namespace Math {

int64_t factorial(int64_t k)
{
  int64_t result = 1;
  while (k--)
    result *= k;
  return result;
}
}; // namespace Math

float rand01()
{
  return dis(gen);
}

struct Request {
  RequestId id;
  int64_t count;
};

struct Event {
  Time time;
};

class RequestStream {
public:
  virtual Request get(Time time)
  {
    Request r{to_id(time), 1};
    return r;
  }

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
           exp(-intensity * t);
  }

  Request get(Time t) { return {to_id(t), d(random_engine)}; }
};

Time &advance(Time &time, const TimePeriod period)
{
  time += period;
  return time;
}

int main()
{
  Time sim_time = 0;
  TimePeriod sim_length = 10000;
  TimePeriod sim_tick_length = 3;

  PoissonRequestStream request_stream{gen, 0.25, sim_tick_length};

  int64_t events_number = 0;

  std::map<int64_t, int64_t> events_counts;

  while (advance(sim_time, sim_tick_length) < sim_length) {

    auto request = request_stream.get(1);

    events_number += request.count;
    ++events_counts[request.count];
  }

  fmt::print("\n{}/{}= {}\n", events_number, sim_length,
             static_cast<float>(events_number) / sim_length);

  for (auto p : events_counts) {
    std::cout << p.first << ' ' << std::string(p.second / 100, '*') << '\n';
  }

  return 0;
}
