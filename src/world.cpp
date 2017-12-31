
#include "world.h"

#include "group.h"
#include "load.h"
#include "logger.h"
#include "source_stream/source_stream.h"

World::World(uint64_t seed, Duration duration, Duration tick_length)
  : seed_(seed), duration_(duration), tick_length_(tick_length)
{
  print("[World] {:=^100}\n", " New world ");
}

void World::init()
{
  for (auto & [ name, source ] : topology_->sources) {
    source->init();
  }
}

bool World::next_iteration()
{
  debug_print("{} Time = {:-<80}\n", *this, time_);

  Time next_event{0};
  if (!events_.empty()) {
    next_event = events_.top()->time;
  }

  time_ += tick_length_;

  time_ = std::max(next_event, time_);

  if (time_ > finish_time_) {
  for (auto & [ name, source ] : topology_->sources) {
  source->pause();
  }
  }

  process_event();
  return time_ <= finish_time_ || !events_.empty();
}

void World::process_event()
{
  while (!events_.empty() && events_.top()->time <= time_) {
    auto &event = events_.top();
    current_time_ = event->time;
    if (!event->skip) {
      debug_print("{} Processing event {}\n", *this, *event);
      event->process();
    } else {
      debug_print("{} Event {} is not processed\n", *this, *event);
      event->skip_notify();
    }
    events_.pop();
  }
}

Uuid World::get_uuid()
{
  return ++last_id;
}

std::mt19937_64 &World::get_random_engine()
{
  return random_engine_;
}

void World::print_stats()
{
  print("{} Time = {}\n", *this, time_);
  print("{} In queue left {} events\n", *this, events_.size());
  for (auto & [ name, group ] : topology_->groups) {
    const auto &group_stats = group->get_stats();
    print("{} Stats for {}: {}\n", *this, *group, group_stats);
    for (auto & [ tc_id, stats ] : group_stats.by_traffic_class) {
      print("{} Stats for {}/{}: {}: {}\n", *this, *group,
            *topology_->find_source_by_tc_id(tc_id).value(),
            topology_->get_traffic_class(tc_id), stats);
    }
  }
}

void World::run(bool quiet)
{
  long double stats_freq = 0.25L;
  int i = 1;
  while (next_iteration()) {
    if (!quiet && get_progress() > stats_freq * i) {
      print_stats();
      ++i;
    }
  }
  if (!quiet) {
    print_stats();
  }
}
void World::set_topology(gsl::not_null<Topology *> topology)
{
  topology_ = make_observer(topology.get());
  topology_->set_world(this); // TODO(PW): rethink this relation
}

void World::schedule(std::unique_ptr<Event> event)
{
  debug_print("{} Scheduled: {}\n", *this, *event);
  events_.emplace(std::move(event));
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const World &world)
{
  f.writer().write("t={} [World]", world.get_current_time());
}
