
#include "world.h"

#include "group.h"
#include "load.h"
#include "logger.h"
#include "simulation/stats_format.h"
#include "source_stream/source_stream.h"

#include <nlohmann/json.hpp>

namespace Simulation
{
World::World(uint64_t seed, Duration duration) : seed_(seed), duration_(duration)
{
  // print("[World] {:=^100}\n", " New world ");
}

void
World::init()
{
  for (auto &[name, source] : topology_->sources) {
    std::ignore = name;
    source->init();
  }
  for (const auto &[id, tc] : topology_->traffic_classes) {
    std::ignore = id;
    blocked_by_size.emplace(tc.size, BlockStats{});
  }
}

bool
World::next_iteration()
{
  debug_print("{} Time = {:-<80}\n", *this, time_);

  Time next_event{0};
  if (!events_.empty()) {
    next_event = events_.top()->time;
  }

  time_ += tick_length_;

  time_ = std::max(next_event, time_);

  if (time_ > finish_time_) {
    for (auto &[name, source] : topology_->sources) {
      std::ignore = name;
      source->pause();
    }
  }

  process_event();
  return time_ <= finish_time_ || !events_.empty();
}

void
World::process_event()
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

Uuid
World::get_uuid()
{
  return ++last_id;
}

std::mt19937_64 &
World::get_random_engine()
{
  return random_engine_;
}

nlohmann::json &
World::append_stats(nlohmann::json &j)
{
  for (auto &[name, group] : topology_->groups) {
    auto &j_group = j[ts::get(name)];

    const auto &group_stats = group->get_stats(Duration{get_time()});
    for (auto &[tc_id, stats] : group_stats.by_traffic_class) {
      auto &j_tc = j_group[std::to_string(ts::get(tc_id))];

      j_tc["served"].push_back(ts::get(stats.lost_served_stats.served.count));
      j_tc["lost"].push_back(ts::get(stats.lost_served_stats.lost.count));
      j_tc["forwarded"].push_back(ts::get(stats.lost_served_stats.forwarded.count));
      j_tc["served_u"].push_back(ts::get(stats.lost_served_stats.served.size));
      j_tc["lost_u"].push_back(ts::get(stats.lost_served_stats.lost.size));
      j_tc["forwarded_u"].push_back(ts::get(stats.lost_served_stats.forwarded.size));
      j_tc["block_time"].push_back(ts::get(stats.block_time));
      j_tc["simulation_time"].push_back(ts::get(stats.simulation_time));
      j_tc["P_loss"].push_back(stats.loss_ratio());
      j_tc["P_loss_u"].push_back(stats.loss_ratio_u());
      j_tc["P_forward"].push_back(stats.forward_ratio());
      j_tc["P_forward_u"].push_back(stats.forward_ratio_u());
      j_tc["P_block"].push_back(stats.block_ratio());
      j_tc["P_block_recursive"].push_back(stats.block_recursive_ratio());
    }
  }
  return j;
}
nlohmann::json
World::get_stats()
{
  nlohmann::json j;
  return append_stats(j);
}

void
World::print_stats()
{
  print("{} Time = {}\n", *this, time_);
  print("{} In queue left {} events\n", *this, events_.size());
  for (auto &[name, group] : topology_->groups) {
    std::ignore = name;
    const auto &group_stats = group->get_stats(Duration{get_time()});
    print("{} {}: {}\n", *this, *group, group_stats);
    for (auto &[tc_id, stats] : group_stats.by_traffic_class) {
      print(
          "{} {} {}: {}: {}\n",
          *this,
          *group,
          *topology_->find_source_by_tc_id(tc_id).value(),
          topology_->get_traffic_class(tc_id),
          stats);
    }
  }

  for (auto &[source_id, source] : topology_->sources) {
    std::ignore = source_id;
    source->print_stats();
  }
}

void
World::run(bool quiet)
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
void
World::set_topology(Topology &topology)
{
  topology_ = &topology;
  topology_->set_world(*this); // TODO(PW): rethink this relation
}

void
World::schedule(std::unique_ptr<Event> event)
{
  debug_print("{} Scheduled: {}\n", *this, *event);
  events_.emplace(std::move(event));
}

void
format_arg(
    fmt::BasicFormatter<char> &f, const char *& /* format_str */, const World &world)
{
  f.writer().write("t={} [World]", world.get_current_time());
}
} // namespace Simulation
