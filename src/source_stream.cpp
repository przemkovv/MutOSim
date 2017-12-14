
#include "group.h"
#include "logger.h"
#include "source_stream.h"

#include <algorithm>
#include <experimental/memory>
#include <iterator>
#include <memory>

using std::experimental::make_observer;

Load SourceStream::get(Time time)
{
  return {world_.get_unique_id(), time, 1, {}, target_group_};
}

EventPtr SourceStream::produce_load(Time time)
{
  Load load{world_.get_unique_id(), time, 1, {}, target_group_};
  return std::make_unique<LoadSendEvent>(world_.get_unique_id(), load);
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

Load PoissonSourceStream::get(Time t)
{
  // auto pf = uniform(world_.get_random_engine());
  // auto dt = -std::log(1.0 - pf) / intensity_ ;
  // auto dt = load_size_/intensity_;

  auto dt = static_cast<Time>(exponential(world_.get_random_engine()));
  auto create_load = [this, t, dt]() -> Load {
    return {world_.get_unique_id(), t + dt,       load_size_, -1, {},
            make_observer(this),    target_group_};
  };
  auto load = create_load();
  debug_print("{} Produced: {}\n", *this, load);

  return load;
}

EventPtr PoissonSourceStream::produce_load(Time time)
{
  auto dt = static_cast<Time>(exponential(world_.get_random_engine()));
  auto create_load = [this, time, dt]() -> Load {
    return {world_.get_unique_id(), time + dt,    load_size_, -1, {},
            make_observer(this),    target_group_};
  };
  auto load = create_load();
  debug_print("{} Produced: {}\n", *this, load);

  auto on_process = [this](World *world, Event *event) {
    auto t = static_cast<LoadSendEvent *>(event)->load.send_time;
    world->schedule(this->produce_load(t));
  };
  return std::make_unique<LoadSendEvent>(world_.get_uuid(), load,
                                         std::move(on_process));
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
Load EngsetSourceStream::get(Time t)
{
  auto params = decltype(exponential)::param_type(
      (sources_number_ - active_sources_) * intensity_);
  exponential.param(params);

  auto dt = static_cast<Time>(exponential(world_.get_random_engine()));
  auto create_load = [this, t, dt]() -> Load {
    return {world_.get_unique_id(), t + dt,       load_size_, -1, {},
            make_observer(this),    target_group_};
  };

  auto load = create_load();
  if (target_group_->can_serve(load)) {
    debug_print("{} Produced: {}\n", *this, load);
    return load;
  }
  return {};
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const EngsetSourceStream &source)
{
  f.writer().write("[EngsetSource {}]", source.id);
}
