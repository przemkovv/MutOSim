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

  bool pause_ = false;

public:
  virtual EventPtr produce_load(Time time);
  virtual void notify_on_serve(const Load &load);
  virtual void init();
  void attach_to_group(gsl::not_null<Group *> target_group);

  SourceStream(World &world) : id(world.get_unique_id()), world_(world) {}
  SourceStream(SourceStream &&) = default;
  SourceStream(const SourceStream &) = default;
  SourceStream &operator=(const SourceStream &) = default;
  SourceStream &operator=(SourceStream &&) = default;
  virtual ~SourceStream() = default;

  void pause() { pause_ = true; }
};

class PoissonSourceStream : public SourceStream
{
  double intensity_;
  Size load_size_;

  std::uniform_real_distribution<> uniform{0.0, 1.0};
  std::exponential_distribution<> exponential{intensity_};

public:
  PoissonSourceStream(World &world, Intensity intensity, Size load_size);

  void init() override;
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
  Size active_sources_ = 0;

  std::exponential_distribution<> exponential{intensity_};

  friend void engset_load_produce_callback(World *world, Event *event);
  friend void format_arg(fmt::BasicFormatter<char> &f,
                         const char *&format_str,
                         const EngsetSourceStream &source);

  Load create_load(Time time);
  std::unique_ptr<LoadProduceEvent> create_produce_load_event(Time time);

public:
  void init() override;
  EventPtr produce_load(Time time) override;
  void notify_on_serve(const Load &load) override;
  EngsetSourceStream(World &world,
                     Intensity intensity,
                     Size sources_number,
                     Size load_size);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const EngsetSourceStream &source);

void engset_load_produce_callback(World *world, Event *event);
