
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
    source->init();
  }
}

bool World::next_iteration()
{
  debug_print("[World] Time = {:-<80} "
              "\n",
              time_);

  Time next_event = 0;
  if (!events_.empty())
    next_event = events_.top()->time;

  if (next_event > time_) {
    time_ = next_event;
  } else {
    debug_print("No events\n");
    time_ += tick_length_;
  }

  process_event();
  return time_ <= duration_;
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
    case EventType::LoadServe: {
      auto serve_event = static_cast<LoadServeEvent *>(event);
      serve_event->load.produced_by->notify_on_serve(serve_event->load);
      break;
    }
    case EventType::LoadProduce:
      break;
    case EventType::None:
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
  debug_print("[World] New load: {}\n", load);
  if (load.target_group) {
    return load.target_group->serve(load);
  } else if (!groups_.empty()) {
    return groups_.front()->serve(load);
  }
  return false;
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
  print("[World] In queue left {} events\n", events_.size());
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
