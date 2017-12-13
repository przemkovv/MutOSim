#pragma once

#include "load.h"
#include "math.h"
#include "types.h"
#include "world.h"

#include <random>

class SourceStream
{
protected:
  Uuid id;
  World &world_;

public:
  virtual Load get(Time time);

  SourceStream(World &world) : id(world.get_unique_id()), world_(world) {}
  SourceStream(SourceStream &&) = default;
  SourceStream(const SourceStream &) = default;
  SourceStream &operator=(const SourceStream &) = default;
  SourceStream &operator=(SourceStream &&) = default;
  virtual ~SourceStream() = default;
};

class PoissonSourceStream : public SourceStream
{
  double intensity_;
  Size load_size_;

  std::uniform_real_distribution<> uniform{0.0, 1.0};
  std::exponential_distribution<> exponential{intensity_};

  friend void format_arg(fmt::BasicFormatter<char> &f,
                         const char *&format_str,
                         const PoissonSourceStream &source);

public:
  PoissonSourceStream(World &world, Intensity intensity, Size load_size);

  Load get(Time t) override;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PoissonSourceStream &source);
