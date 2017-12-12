#pragma once

#include "math.h"
#include "request.h"
#include "types.h"
#include "world.h"

#include <random>

class SourceStream
{
protected:
  World &world_;

public:
  virtual Request get(Time time);

  SourceStream(World &world) : world_(world) {}
  SourceStream(SourceStream &&) = default;
  SourceStream(const SourceStream &) = default;
  SourceStream &operator=(const SourceStream &) = default;
  SourceStream &operator=(SourceStream &&) = default;
  virtual ~SourceStream() = default;
};

class PoissonSourceStream : public SourceStream
{
  double intensity_;
  TimePeriod time_period_;

  std::poisson_distribution<int64_t> d_;

  double Pk(const int k, const Time t);

public:
  PoissonSourceStream(World &world, double intensity, TimePeriod time_period);

  Request get(Time t) override;
};
