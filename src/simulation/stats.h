#pragma once

#include "math_utils.h"
#include "simulation/load.h"
#include "types/types.h"

#include <boost/container/flat_map.hpp>
#include <map>
#include <unordered_map>

namespace Simulation {
struct LoadStats
{
  Count count;
  Size  size;
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

  void serve(const Load &load)
  {
    if (load.compression_ratio == nullptr)
    {
      served.size += load.size;
    }
    else
    {
      // TODO(PW): make it more accurate (casting to int)
      served.size += Size{load.compression_ratio->size
                          / load.compression_ratio->intensity_factor};
    }
    served.count++;
  }
  void drop(const Load &load)
  {
    lost.size += load.size;
    lost.count++;
  }
  void forward(const Load &load)
  {
    forwarded.size += load.size;
    forwarded.count++;
  }
};
//----------------------------------------------------------------------

struct TrafficClassStats
{
  LostServedStats lost_served_stats{};
  Duration        block_time{};
  Duration        block_recursive_time{};
  Duration        simulation_time{};

  double loss_ratio() const
  {
    return Math::ratio_to_sum<double>(
        get(lost_served_stats.lost.count),
        get(lost_served_stats.served.count),
        get(lost_served_stats.forwarded.count));
  }
  double loss_ratio_u() const
  {
    return Math::ratio_to_sum<double>(
        get(lost_served_stats.lost.size),
        get(lost_served_stats.served.count),
        get(lost_served_stats.forwarded.count));
  }
  double forward_ratio() const
  {
    return Math::ratio_to_sum<double>(
        get(lost_served_stats.forwarded.count),
        get(lost_served_stats.served.count),
        get(lost_served_stats.lost.count));
  }
  double forward_ratio_u() const
  {
    return Math::ratio_to_sum<double>(
        get(lost_served_stats.forwarded.size),
        get(lost_served_stats.served.count),
        get(lost_served_stats.lost.count));
  }
  auto block_ratio() const { return block_time / simulation_time; }
  auto block_recursive_ratio() const
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
