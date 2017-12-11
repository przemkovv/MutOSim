
#include <fmt/format.h>
#include <iostream>
#include <optional>

using Time = int64_t;
using TimeInterval = int64_t;
using RequestId = uint64_t;

template <typename T>
RequestId to_id(T value) {
  return static_cast<RequestId>(value);
}

namespace Math {

  int64_t factorial(int64_t k) {
    int64_t result = 1;
    while (k--) result *=k;
    return result;
  }
};

float rand01()
{
  return 0.5;
}

struct Request {
  RequestId id;
};

struct Event {
  Time time;
};

struct RequestStream {
  virtual std::optional<Request> get(Time time)
  {
    Request r { to_id(time) };
    return r;
  }

  virtual ~RequestStream() = default;

};

struct PoissonRequestStream : public RequestStream {
  float intensity = 0.5;;
  TimeInterval delta_time = 1;

  Time time = 0;

  float theta(const TimeInterval delta_time) { return 0.0f; }

  float Pk(const int k, const Time t)
  {
    return std::pow(intensity * t, k) / Math::factorial(k) *
           exp(-intensity * t);
  }
  float P0(const Time t) { return Pk(0, t); }
  float P1(const Time t) { return Pk(1, t); }

  std::optional<Request> get(Time t)
  {
    if (rand01() > P0(t))
      return {};
    else
      return Request{to_id(t)};
  }
};

int main()
{
  Time sim_time = 0;
  TimeInterval sim_length = 10;

  PoissonRequestStream request_stream;
  request_stream.intensity = 0.2;

  while (sim_time++ < sim_length) {

    auto request = request_stream.get(sim_time);

    fmt::print("{0}: have request? {1}\n", sim_time, static_cast<bool>(request));
  }

  return 0;
}
