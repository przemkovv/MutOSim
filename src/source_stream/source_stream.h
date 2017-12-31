#pragma once

#include "event.h"
#include "traffic_class.h"
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
  const TrafficClass tc_;

protected:
  observer_ptr<World> world_;

  observer_ptr<Group> target_group_;

  bool pause_ = false;

  Load create_load(Time time, Size size);

public:
  virtual void init();
  virtual void notify_on_service_start(const LoadServiceRequestEvent *event);
  virtual void notify_on_service_end(const LoadServiceEndEvent *event);
  virtual void notify_on_service_accept(const LoadServiceRequestEvent *event);
  virtual void notify_on_service_drop(const LoadServiceRequestEvent *event);
  virtual void notify_on_produce(const ProduceServiceRequestEvent *event);

  virtual Size get_load_size() const { return tc_.size; }
  virtual Intensity get_intensity() const { return tc_.serve_intensity; }

  SourceStream(const SourceName &name, const TrafficClass &tc) : name_(name), tc_(tc) {}
  virtual ~SourceStream() = default;

  void set_world(gsl::not_null<World *> world);
  void attach_to_group(gsl::not_null<Group *> target_group);
  void pause() { pause_ = true; }

  const SourceName &get_name() { return name_; }

  friend void format_arg(fmt::BasicFormatter<char> &f,
                         const char *& /* format_str */,
                         const SourceStream &source);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const SourceStream &source);
