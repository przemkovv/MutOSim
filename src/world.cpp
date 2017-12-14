
#include "world.h"

#include "group.h"
#include "load.h"
#include "logger.h"
#include "source_stream.h"

World::World(uint64_t seed, Duration duration, Duration tick_length)
  : seed_(seed), duration_(duration), tick_length_(tick_length)
{
  print("[World] {:=^100}\n", " New world ");
}

World::~World()
{
  print("[World] {:^^100}\n", " End of the world ");
}
void World::init()
{
  for (auto &source : sources_) {
    schedule(source->produce_load(time_));
  }
}

bool World::next_iteration()
{
  debug_print("[World] Time = {} "
              "----------------------------------------------------------------"
              "\n",
              time_);

  process_event();

  Time next_event = 0;
  if (!events_.empty())
    next_event = events_.top()->time;

  if (next_event > time_) {
    time_ = next_event;
  } else {
    print("No events\n");
    time_ += tick_length_;
  }

  return time_ <= duration_;
}

void World::queue_load_to_serve(Load load)
{
  loads_served_.emplace(load);
}

void World::process_event()
{
  if (!events_.empty()) {
    auto event = events_.top().get();
    switch (event->type) {
    case EventType::LoadSend: {
      auto send_event = static_cast<LoadSendEvent *>(event);
      serve_load(send_event->load);
      break;
    }
    case EventType::LoadServe:
      break;
    case EventType::LoadProduce:
      break;
    }

    if (event->on_process) {
      event->on_process(this, event);
    }

    events_.pop();
  }
}

bool World::serve_load(Load load)
{
  if (load.target_group) {
    return load.target_group->serve(load);
  } else if (!groups_.empty()) {
    debug_print("[World] New load: {}\n", load);
    return groups_.front()->serve(load);
  }
  return false;
}

void World::send_loads()
{
  while (!loads_send_.empty() && loads_send_.top().send_time <= time_) {
    auto load = loads_send_.top();
    loads_send_.pop();
    loads_send_.emplace(load.produced_by->get(load.send_time));
    serve_load(load);
  }
}

void World::serve_loads()
{
  while (!loads_served_.empty() && loads_served_.top().end_time <= time_) {
    auto &load = loads_served_.top();
    load.served_by->take_off(load);
    loads_served_.pop();
  }
}

// TODO(PW): remove
Uuid World::get_unique_id()
{
  return ++last_id;
}

Uuid World::get_uuid()
{
  return ++last_id;
}

std::mt19937_64 &World::get_random_engine()
{
  return random_engine_;
}

void World::add_group(gsl::not_null<Group *> group)
{
  groups_.emplace_back(group);
}

void World::add_source(gsl::not_null<SourceStream *> source)
{
  sources_.emplace_back(source);
}
void World::print_stats()
{
  print("[World] Left in queue {}\n", loads_served_.size());
  print("[World] Time = {:f}\n", time_);
  for (auto &group : groups_) {
    print("[World] Stats for {}: {}\n", *group, group->get_stats());
  }
}

void World::run()
{
  double stats_freq = 0.2;
  int i = 1;
  while (next_iteration()) {
    if (get_progress() > stats_freq * i) {
      print_stats();
      ++i;
    }
  }
  print_stats();
}
