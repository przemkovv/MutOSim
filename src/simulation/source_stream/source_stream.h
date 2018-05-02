#pragma once

#include "simulation/event.h"
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
  World *world_ = nullptr;

  Group *target_group_ = nullptr;

  bool pause_ = false;
  uint64_t loads_produced_{0};

  Load create_load(Time time, Size size);

public:
  virtual void init();
  virtual void notify_on_service_start(const LoadServiceRequestEvent *event);
  virtual void notify_on_service_end(const LoadServiceEndEvent *event);
  virtual void notify_on_service_accept(const LoadServiceRequestEvent *event);
  virtual void notify_on_service_drop(const LoadServiceRequestEvent *event);
  virtual void notify_on_produce(const ProduceServiceRequestEvent *event);
  virtual void notify_on_skip_processing(const Event *event);

  virtual Size get_load_size() const { return tc_.size; }
  virtual Intensity get_intensity() const { return tc_.serve_intensity; }

  SourceStream(const SourceName &name, const TrafficClass &tc);
  SourceStream(const SourceStream &) = delete;
  SourceStream &operator=(const SourceStream &) = delete;
  virtual ~SourceStream() = default;

  void set_world(World &world);
  void attach_to_group(Group &target_group);
  void pause() { pause_ = true; }

  const SourceName &get_name() { return name_; }
  const Group &get_target_group() { return *target_group_; }

  void print_stats();

  friend void format_arg(
      fmt::BasicFormatter<char> &f,
      const char *& /* format_str */,
      const SourceStream &source);
};

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const SourceStream &source);
