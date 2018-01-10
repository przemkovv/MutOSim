
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

nlohmann::json World::get_stats()
{

  nlohmann::json j;

  for (auto &[name, group] : topology_->groups) {
    nlohmann::json j_group ={};

    const auto &group_stats = group->get_stats();
    for (auto &[tc_id, stats] : group_stats.by_traffic_class) {
      nlohmann::json j_tc = {};

      j_tc["served"] = ts::get(stats.lost_served_stats.served.count);
      j_tc["lost"] = ts::get(stats.lost_served_stats.lost.count);
      j_tc["served_u"] = ts::get(stats.lost_served_stats.served.size);
      j_tc["lost_u"] = ts::get(stats.lost_served_stats.lost.size);
      j_tc["block_time"] = ts::get(stats.block_time);
      j_tc["simulation_time"] = ts::get(stats.simulation_time);
      j_tc["P_loss"] = stats.loss_ratio();
      j_tc["P_loss_u"] = stats.loss_ratio_u();
      j_tc["P_block"] = stats.block_ratio();

      j_group[std::to_string(ts::get(tc_id))] = j_tc;
    }

    j[ts::get(name)] = j_group;
  }
  return j;
}

void World::print_stats()
{
  print("{} Time = {}\n", *this, time_);
  print("{} In queue left {} events\n", *this, events_.size());
  for (auto &[name, group] : topology_->groups) {
    const auto &group_stats = group->get_stats();
    print("{} Stats for {}: {}\n", *this, *group, group_stats);
    for (auto &[tc_id, stats] : group_stats.by_traffic_class) {
      print("{} Stats for {}/{}: {}: {}\n", *this, *group,
            *topology_->find_source_by_tc_id(tc_id).value(),
            topology_->get_traffic_class(tc_id), stats);
    }
  }
  for (auto &tc : topology_->traffic_classes) {
    print("{} Stats for {}: P_block {}\n", *this, tc,
          blocked_by_tc[tc.id].block_time / Duration{ts::get(current_time_)});
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
  for (const auto &tc : topology_->traffic_classes) {
    if (std::any_of(
            begin(topology_->groups), end(topology_->groups),
            [&tc](const auto &group) { return group.second->can_serve(tc.size); })) {
      unblock(tc.id, load);
    } else {
      block(tc.id, load);
    }
  }
}
void World::block(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = blocked_by_tc[tc_id];
  if (!block_stats.is_blocked) {
    block_stats.is_blocked = true;
    block_stats.start_of_block = load.send_time;
  }
}

void World::unblock(TrafficClassId tc_id, const Load &load)
{
  auto &block_stats = blocked_by_tc[tc_id];
  if (block_stats.is_blocked) {
    block_stats.is_blocked = false;
    auto block_time = load.end_time - block_stats.start_of_block;
    block_stats.block_time += block_time;
  }
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const World &world)
{
  f.writer().write("t={} [World]", world.get_current_time());
}
