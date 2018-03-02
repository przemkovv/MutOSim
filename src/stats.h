#pragma once

#include "math.h"
#include "types.h"
#include "load.h"

#include <fmt/format.h>
#include <map>
#include <unordered_map>

struct LoadStats {
  Count count;
  Size size;
};

struct BlockStats {
  Duration block_time{0};
  Time start_of_block{0};
  bool is_blocked = false;

  bool try_block(const Time &time);
  bool try_unblock(const Time &time);
};

struct LostServedStats {
  LoadStats lost{};
  LoadStats served{};
  LoadStats forwarded{};

  void serve(const Load &load)
  {
    served.size += load.size;
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

struct TrafficClassStats {
  LostServedStats lost_served_stats{};
  Duration block_time{};
  Duration simulation_time{};

  double loss_ratio() const
  {
    return Math::ratio_to_sum<double>(get(lost_served_stats.lost.count),
                                      get(lost_served_stats.served.count),
                                      get(lost_served_stats.forwarded.count));
  }
  double loss_ratio_u() const
  {
    return Math::ratio_to_sum<double>(get(lost_served_stats.lost.size),
                                      get(lost_served_stats.served.count),
                                      get(lost_served_stats.forwarded.count));
  }
  double forward_ratio() const
  {
    return Math::ratio_to_sum<double>(get(lost_served_stats.forwarded.count),
                                      get(lost_served_stats.served.count),
                                      get(lost_served_stats.lost.count));
  }
  double forward_ratio_u() const
  {
    return Math::ratio_to_sum<double>(get(lost_served_stats.forwarded.size),
                                      get(lost_served_stats.served.count),
                                      get(lost_served_stats.lost.count));
  }
  auto block_ratio() const { return block_time / simulation_time; }
};

struct Stats {
  LostServedStats total{};
  std::map<TrafficClassId, TrafficClassStats> by_traffic_class{};
};

LoadStats operator+(const LoadStats &s1, const LoadStats &s2);
LoadStats &operator+=(LoadStats &s1, const LoadStats &s2);
LostServedStats operator+(const LostServedStats &s1, const LostServedStats &s2);
LostServedStats &operator+=(LostServedStats &s1, const LostServedStats &s2);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Stats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const TrafficClassStats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LoadStats &load_stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const std::unordered_map<SourceId, LoadStats> &served_by_traffic_class);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LostServedStats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const std::map<TrafficClassId, LostServedStats> &lost_served_stats);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const std::unordered_map<TrafficClassId, LostServedStats> &lost_served_stats);
