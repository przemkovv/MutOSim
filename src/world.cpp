
#include "world.h"

#include "group.h"
#include "load.h"
#include "logger.h"
#include "source_stream/source_stream.h"

World::World(uint64_t seed, Duration duration, Duration tick_length)
  : seed_(seed), duration_(duration), tick_length_(tick_length)
{
  // print("[World] {:=^100}\n", " New world ");
}

void World::init()
{
  for (auto &[name, source] : topology_->sources) {
    source->init();
  }
  for (const auto &[id, tc] : topology_->traffic_classes) {
    blocked_by_size.emplace(tc.size, BlockStats{});
  }
}

void World::reset() {
  blocked_by_tc.clear();
  blocked_by_size.clear();
  time_ = Time{0};
  current_time_ = Time{0};
  last_id = Uuid{0};
  events_ = decltype(events_){}; // NOTE(PW): std::priority_queue doesn't have clear

  for (auto &[name, source] : topology_->sources) {
    source->reset();
  }
  for (auto &[name, group] : topology_->groups) {
    group->reset();
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
    for (auto &[name, source] : topology_->sources) {
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

nlohmann::json World::append_stats(nlohmann::json &j)
{
  for (auto &[name, group] : topology_->groups) {
    auto &j_group = j[ts::get(name)];

    const auto &group_stats = group->get_stats();
    for (auto &[tc_id, stats] : group_stats.by_traffic_class) {
      auto &j_tc = j_group[std::to_string(ts::get(tc_id))];

      j_tc["served"].push_back(ts::get(stats.lost_served_stats.served.count));
      j_tc["lost"].push_back(ts::get(stats.lost_served_stats.lost.count));
      j_tc["served_u"].push_back(ts::get(stats.lost_served_stats.served.size));
      j_tc["lost_u"].push_back(ts::get(stats.lost_served_stats.lost.size));
      j_tc["block_time"].push_back(ts::get(stats.block_time));
      j_tc["simulation_time"].push_back(ts::get(stats.simulation_time));
      j_tc["P_loss"].push_back(stats.loss_ratio());
      j_tc["P_loss_u"].push_back(stats.loss_ratio_u());
      j_tc["P_block"].push_back(stats.block_ratio());
    }
  }
  return j;
}
nlohmann::json World::get_stats()
{
  nlohmann::json j;

  return append_stats(j);
  // for (auto &[name, group] : topology_->groups) {
    // auto &j_group = j[ts::get(name)];

    // const auto &group_stats = group->get_stats();
    // for (auto &[tc_id, stats] : group_stats.by_traffic_class) {
      // auto &j_tc = j_group[std::to_string(ts::get(tc_id))];

      // j_tc["served"] = ts::get(stats.lost_served_stats.served.count);
      // j_tc["lost"] = ts::get(stats.lost_served_stats.lost.count);
      // j_tc["served_u"] = ts::get(stats.lost_served_stats.served.size);
      // j_tc["lost_u"] = ts::get(stats.lost_served_stats.lost.size);
      // j_tc["block_time"] = ts::get(stats.block_time);
      // j_tc["simulation_time"] = ts::get(stats.simulation_time);
      // j_tc["P_loss"] = stats.loss_ratio();
      // j_tc["P_loss_u"] = stats.loss_ratio_u();
      // j_tc["P_block"] = stats.block_ratio();
    // }

    // // j[ts::get(name)] = j_group;
  // }
  // return j;
}

void World::print_stats()
{
  print("{} Time = {}\n", *this, time_);
  print("{} In queue left {} events\n", *this, events_.size());
  for (auto &[name, group] : topology_->groups) {
    const auto &group_stats = group->get_stats();
    print("{} {}: {}\n", *this, *group, group_stats);
    for (auto &[tc_id, stats] : group_stats.by_traffic_class) {
      print("{} {} {}: {}: {}\n", *this, *group,
            *topology_->find_source_by_tc_id(tc_id).value(),
            topology_->get_traffic_class(tc_id), stats);
    }
  }
  for (auto &[tc_id, tc] : topology_->traffic_classes) {
    auto p_block = blocked_by_tc[tc.id].block_time / Duration{ts::get(current_time_)};
    print("{} {}: P_block {:<12} ({:<12})\n", *this, tc, p_block, std::log10(p_block));
  }

  for (auto &[size, stats] : blocked_by_size) {
    auto p_block = stats.block_time / Duration{ts::get(current_time_)};
    print("{} Size: {}: P_block {:<12} ({:<12})\n", *this, size, p_block,
          std::log10(p_block));
  }
  for (auto &[source_id, source] : topology_->sources) {
    source->print_stats();
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

void World::update_block_stat(const Load &load)
{
  auto can_serve = [](Size size) {
    return [size](const auto &group) { return group.second->can_serve(size); };
  };
  // TODO(PW): think how to measure global block
  for (const auto &[tc_id, tc] : topology_->traffic_classes) {
    if (std::none_of(begin(topology_->groups), end(topology_->groups),
                     can_serve(tc.size))) {
      blocked_by_tc[tc_id].try_block(load.send_time);
    }
  }
  for (auto &[size, stats] : blocked_by_size) {
    if (std::none_of(begin(topology_->groups), end(topology_->groups), can_serve(size))) {
      stats.try_block(load.send_time);
    }
  }
}
void World::update_unblock_stat(const Load &load)
{
  auto can_serve = [](Size size) {
    return [size](const auto &group) { return group.second->can_serve(size); };
  };
  // TODO(PW): think how to measure global block
  for (const auto &[tc_id, tc] : topology_->traffic_classes) {
    if (std::any_of(begin(topology_->groups), end(topology_->groups),
                    can_serve(tc.size))) {
      blocked_by_tc[tc_id].try_unblock(load.end_time);
    }
  }
  for (auto &[size, stats] : blocked_by_size) {
    if (std::any_of(begin(topology_->groups), end(topology_->groups), can_serve(size))) {
      stats.try_unblock(load.end_time);
    }
  }
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const World &world)
{
  f.writer().write("t={} [World]", world.get_current_time());
}
