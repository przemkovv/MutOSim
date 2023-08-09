#include "stats.h"

#include "math_utils.h"

namespace Simulation {
//----------------------------------------------------------------------

LoadStats
operator+(const LoadStats &s1, const LoadStats &s2)
{
  return {s1.count + s2.count, s1.size + s2.size};
}

LoadStats &
operator+=(LoadStats &s1, const LoadStats &s2)
{
  s1.count += s2.count;
  s1.size += s2.size;
  return s1;
}

LostServedStats
operator+(const LostServedStats &s1, const LostServedStats &s2)
{
  return {
      s1.lost + s2.lost, s1.served + s2.served, s1.forwarded + s2.forwarded};
}

LostServedStats &
operator+=(LostServedStats &s1, const LostServedStats &s2)
{
  s1.lost += s2.lost;
  s1.served += s2.served;
  s1.forwarded += s2.forwarded;
  return s1;
}

//----------------------------------------------------------------------

bool
BlockStats::try_block(const Time &time)
{
  if (!is_blocked)
  {
    is_blocked = true;
    start_of_block = time;
    return true;
  }
  return false;
}

bool
BlockStats::try_unblock(const Time &time)
{
  if (is_blocked)
  {
    is_blocked = false;
    block_time += time - start_of_block;
    return true;
  }
  return false;
}
//----------------------------------------------------------------------

void
LostServedStats::serve(const Load &load)
{
  if (load.compression_ratio == nullptr)
  {
    served.size += load.size;
  }
  else
  {
    // TODO(PW): make it more accurate (casting to int)
    auto foo =
        load.compression_ratio->size / load.compression_ratio->intensity_factor;
    served.size += Simulation::Size{foo};
  }
  served.count++;
}

void
LostServedStats::drop(const Load &load)
{
  lost.size += load.size;
  lost.count++;
}
void
LostServedStats::forward(const Load &load)
{
  forwarded.size += load.size;
  forwarded.count++;
}

//----------------------------------------------------------------------

double
TrafficClassStats::loss_ratio() const
{
  return Math::ratio_to_sum<double>(
      static_cast<double>(get(lost_served_stats.lost.count)),
      get(lost_served_stats.served.count),
      get(lost_served_stats.forwarded.count));
}
double
TrafficClassStats::loss_ratio_u() const
{
  return Math::ratio_to_sum<double>(
      static_cast<double>(get(lost_served_stats.lost.size)),
      get(lost_served_stats.served.count),
      get(lost_served_stats.forwarded.count));
}
double
TrafficClassStats::forward_ratio() const
{
  return Math::ratio_to_sum<double>(
      static_cast<double>(get(lost_served_stats.forwarded.count)),
      get(lost_served_stats.served.count),
      get(lost_served_stats.lost.count));
}
double
TrafficClassStats::forward_ratio_u() const
{
  return Math::ratio_to_sum<double>(
      static_cast<double>(get(lost_served_stats.forwarded.size)),
      get(lost_served_stats.served.count),
      get(lost_served_stats.lost.count));
}
//----------------------------------------------------------------------
Stats
GroupStatistics::get_stats(Duration sim_duration)
{
  Stats stats;

  for (auto &[tc_id, load_stats] : served_by_tc)
  {
    std::ignore = load_stats;
    auto &serve_stats = served_by_tc[tc_id];
    if (serve_stats.lost.count == Count{0}
        && serve_stats.served.count == Count{0})
      continue;
    stats.by_traffic_class[tc_id] = {
        {serve_stats.lost, serve_stats.served, serve_stats.forwarded},
        blocked_by_tc[tc_id].block_time,
        blocked_recursive_by_tc[tc_id].block_time,
        sim_duration};
    stats.total += serve_stats;
  }
  return stats;
}

} // namespace Simulation
