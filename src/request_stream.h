
#include "types.h"
#include "request.h"
#include "math.h"

#include <random>

class RequestStream
{
public:
  virtual Request get(Time time);

  RequestStream() = default;
  RequestStream(RequestStream &&) = default;
  RequestStream(const RequestStream &) = default;
  RequestStream &operator=(const RequestStream &) = default;
  RequestStream &operator=(RequestStream &&) = default;
  virtual ~RequestStream() = default;
};

template <typename RandomEngine>
class PoissonRequestStream : public RequestStream
{
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
