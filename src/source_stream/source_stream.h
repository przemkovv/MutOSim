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
  const SourceName name_;
  SourceId id{0};

protected:
  observer_ptr<World> world_;

  observer_ptr<Group> target_group_;

  bool pause_ = false;

  Load create_load(Time time, Size size);

public:
  virtual void init();
  virtual void notify_on_send(const LoadSendEvent *event);
  virtual void notify_on_serve(const LoadServeEvent *event);
  virtual void notify_on_accept(const LoadSendEvent *event);
  virtual void notify_on_produce(const LoadProduceEvent *event);

  virtual Size get_load_size() = 0;
  virtual Intensity get_intensity() = 0;

  SourceStream(const SourceName &name) : name_(name) {}
  virtual ~SourceStream() = default;

  void set_world(gsl::not_null<World *> world);
  void attach_to_group(gsl::not_null<Group *> target_group);
  void pause() { pause_ = true; }

  const SourceName &get_name() { return name_; }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const SourceStream &source);
