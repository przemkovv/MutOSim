#pragma once

#include "math.h"
#include "request.h"
#include "types.h"

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
  RandomEngine &random_engine_;

  float intensity_;
  TimePeriod time_period_;

  std::poisson_distribution<int64_t> d_;

public:
  PoissonRequestStream(RandomEngine &random_engine,
                       float intensity,
                       TimePeriod time_period)
    : random_engine_(random_engine),
      intensity_(intensity),
      time_period_(time_period),
      d_(static_cast<double>(intensity_) * time_period_)
  {
  }

  float Pk(const int k, const Time t)
  {
    return std::pow(intensity_ * t, k) / Math::factorial(k) *
           expf(-intensity_ * t);
  }

  Request get(Time t) override { return {to_id(t), d_(random_engine_)}; }
};
