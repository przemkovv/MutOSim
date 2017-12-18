#pragma once

#include "event.h"
#include "types.h"

#include <fmt/format.h>
#include <memory>

class World;
struct Group;

class SourceStream
{
public:
  const Name name_;
  Uuid id = 0;

protected:
  observer_ptr<World> world_;

  observer_ptr<Group> target_group_;

  bool pause_ = false;

public:
  void set_world(gsl::not_null<World *> world);

  virtual EventPtr produce_load(Time time);
  virtual void notify_on_serve(const Load &load);
  virtual void notify_on_accept(const Load &load);
  virtual void notify_on_produce(const LoadProduceEvent *produce_event);
  virtual void init();
  void attach_to_group(gsl::not_null<Group *> target_group);
  const Name &get_name() { return name_; }

  SourceStream(const Name &name) : name_(name) {}
  SourceStream(SourceStream &&) = default;
  SourceStream(const SourceStream &) = default;
  SourceStream &operator=(const SourceStream &) = default;
  SourceStream &operator=(SourceStream &&) = default;
  virtual ~SourceStream() = default;

  void pause() { pause_ = true; }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const SourceStream &source);
