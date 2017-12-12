#pragma once

#include "load.h"
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
  virtual Load get(Time time);

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
  Size load_size_;

  std::poisson_distribution<uint64_t> d_;
  std::uniform_real_distribution<> uniform{0.0, 1.0};

  double Pk(const int k, const Time t);

public:
  PoissonSourceStream(World &world,
                      double intensity,
                      Size load_size,
                      TimePeriod time_period);

  Load get(Time t) override;
};
