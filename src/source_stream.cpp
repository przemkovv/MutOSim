
#include "group.h"
#include "logger.h"
#include "source_stream.h"

#include <algorithm>
#include <experimental/memory>
#include <iterator>
#include <memory>

using std::experimental::make_observer;

EventPtr SourceStream::produce_load(Time time)
{
  Load load{world_.get_unique_id(), time, 1, {}, target_group_};
  return std::make_unique<LoadSendEvent>(world_.get_unique_id(), load);
}

void SourceStream::notify_on_serve(const Load & /* load */)
{
}

void SourceStream::init()
{
}

void SourceStream::attach_to_group(gsl::not_null<Group *> target_group)
{
  target_group_ = make_observer(target_group.get());
}

PoissonSourceStream::PoissonSourceStream(World &world,
                                         Intensity intensity,
                                         Size load_size)
  : SourceStream(world), intensity_(intensity), load_size_(load_size)
{
}

static void poisson_produce_load_callback(World *world, Event *event)
{
  auto send_event = static_cast<LoadSendEvent *>(event);
  auto &produced_by = send_event->load.produced_by;
  world->schedule(produced_by->produce_load(send_event->load.send_time));
}

void PoissonSourceStream::init()
{
  world_.schedule(produce_load(world_.get_time()));
}

EventPtr PoissonSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_.get_uuid(), time);
  }
  auto dt = static_cast<Time>(exponential(world_.get_random_engine()));
  auto create_load = [this, time, dt]() -> Load {
    return {world_.get_unique_id(), time + dt,    load_size_, -1, {},
            make_observer(this),    target_group_};
  };
  auto load = create_load();
  debug_print("{} Produced: {}\n", *this, load);

  // auto on_process = [this](World *world, Event *event) {
  // auto t = static_cast<LoadSendEvent *>(event)->load.send_time;
  // world->schedule(this->produce_load(t));
  // };
  return std::make_unique<LoadSendEvent>(world_.get_uuid(), load,
                                         poisson_produce_load_callback);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const PoissonSourceStream &source)
{
  f.writer().write("[PoissonSource {}]", source.id);
}

EngsetSourceStream::EngsetSourceStream(World &world,
                                       Intensity intensity,
                                       Size sources_number,
                                       Size load_size)
  : SourceStream(world),
    intensity_(intensity),
    load_size_(load_size),
    sources_number_(sources_number)
{
}

void engset_load_produce_callback(World *world, Event *event)
{
  auto produce_event = static_cast<LoadProduceEvent *>(event);
  auto &source_stream = produce_event->source_stream;
  world->schedule(source_stream->produce_load(event->time));
}

void EngsetSourceStream::notify_on_serve(const Load &load)
{
  active_sources_--;
  debug_print("{} Load has been served {}\n", *this, load);

  world_.schedule(create_produce_load_event(load.end_time));
}

void EngsetSourceStream::init()
{
  for (Size i = 0; i < sources_number_; ++i) {
    world_.schedule(create_produce_load_event(world_.get_time()));
  }
}
Load EngsetSourceStream::create_load(Time time)
{
  return {world_.get_unique_id(), time,         load_size_, -1, {},
          make_observer(this),    target_group_};
}

std::unique_ptr<LoadProduceEvent>
EngsetSourceStream::create_produce_load_event(Time time)
{
  // auto params = decltype(exponential)::param_type(
  // (sources_number_ - active_sources_) * intensity_);
  // exponential.param(params);
  auto dt = static_cast<Time>(exponential(world_.get_random_engine()));
  return std::make_unique<LoadProduceEvent>(world_.get_uuid(), time + dt, this,
                                            engset_load_produce_callback);
}

EventPtr EngsetSourceStream::produce_load(Time time)
{
  if (pause_) {
    return std::make_unique<Event>(EventType::None, world_.get_uuid(), time);
  }
  auto load = create_load(time);
  if (target_group_->can_serve(load.size)) {
    debug_print("{} Produced: {}\n", *this, load);
    active_sources_++;
  } else {
    debug_print("{} Produced ghost: {}\n", *this, load);
    world_.schedule(create_produce_load_event(time));
  }
  return std::make_unique<LoadSendEvent>(world_.get_uuid(), load);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EngsetSourceStream &source)
{
  f.writer().write(
      "[EngsetSource {}, active={}/{}, gamma={}, lambda={}]", source.id,
      source.active_sources_, source.sources_number_, source.intensity_,
      (source.sources_number_ - source.active_sources_) * source.intensity_);
}
