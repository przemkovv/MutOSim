#pragma once

#include "simulation/load.h"
#include "types/types.h"

#include <boost/container/flat_map.hpp>
#include <map>
#include <unordered_map>

namespace Simulation {

struct LoadStats
{
  Simulation::Count count;
  Simulation::Size  size;
};

//----------------------------------------------------------------------
struct BlockStats
{
  Duration block_time{0};
  Time     start_of_block{0};
  bool     is_blocked = false;

  bool try_block(const Time &time);
  bool try_unblock(const Time &time);
};
//----------------------------------------------------------------------

struct LostServedStats
{
  LoadStats lost{};
  LoadStats served{};
  LoadStats forwarded{};

  void serve(const Load &load);
  void drop(const Load &load);
  void forward(const Load &load);
};
//----------------------------------------------------------------------

struct TrafficClassStats
{
  LostServedStats lost_served_stats{};
  Duration        block_time{};
  Duration        block_recursive_time{};
  Duration        simulation_time{};

  double loss_ratio() const;
  double loss_ratio_u() const;
  double forward_ratio() const;
  double forward_ratio_u() const;
  auto   block_ratio() const { return block_time / simulation_time; }
  auto   block_recursive_ratio() const
  {
    return block_recursive_time / simulation_time;
  }
};

//----------------------------------------------------------------------
struct Stats
{
  LostServedStats                             total{};
  std::map<TrafficClassId, TrafficClassStats> by_traffic_class{};
};

//----------------------------------------------------------------------
struct GroupStatistics
{
  boost::container::flat_map<TrafficClassId, LostServedStats> served_by_tc;
  boost::container::flat_map<TrafficClassId, BlockStats>      blocked_by_tc;
  boost::container::flat_map<TrafficClassId, BlockStats>
      blocked_recursive_by_tc;

  Stats get_stats(Duration sim_duration);
};

//----------------------------------------------------------------------
LoadStats       operator+(const LoadStats &s1, const LoadStats &s2);
LoadStats &     operator+=(LoadStats &s1, const LoadStats &s2);
LostServedStats operator+(const LostServedStats &s1, const LostServedStats &s2);
LostServedStats &operator+=(LostServedStats &s1, const LostServedStats &s2);

} // namespace Simulation
