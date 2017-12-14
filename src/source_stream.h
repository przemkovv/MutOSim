#pragma once

#include "load.h"
#include "math.h"
#include "types.h"
#include "world.h"

#include <experimental/memory>
#include <gsl/gsl>
#include <random>

using std::experimental::observer_ptr;

class SourceStream
{
public:
  const Uuid id;

protected:
  World &world_;

  observer_ptr<Group> target_group_;

public:
  virtual Load get(Time time);
  virtual EventPtr produce_load(Time time);
  void attach_to_group(gsl::not_null<Group *> target_group);

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

public:
  PoissonSourceStream(World &world, Intensity intensity, Size load_size);

  Load get(Time t) override;
  EventPtr produce_load(Time time) override;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PoissonSourceStream &source);

class EngsetSourceStream : public SourceStream
{
  double intensity_;
  Size load_size_;
  Size sources_number_;
  Size active_sources_ =0;

  std::exponential_distribution<> exponential{intensity_};

public:
  EngsetSourceStream(World &world,
                     Intensity intensity,
                     Size sources_number,
                     Size load_size);

  Load get(Time t) override;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EngsetSourceStream &source);
