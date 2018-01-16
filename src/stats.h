#pragma once

#include "math.h"
#include "types.h"

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
  LoadStats lost;
  LoadStats served;
};

struct TrafficClassStats {
  LostServedStats lost_served_stats;
  Duration block_time;
  Duration simulation_time;

  double loss_ratio() const
  {
    return Math::ratio_to_sum<double>(ts::get(lost_served_stats.lost.count),
                                      ts::get(lost_served_stats.served.count));
  }
  double loss_ratio_u() const
  {
    return Math::ratio_to_sum<double>(ts::get(lost_served_stats.lost.size),
                                      ts::get(lost_served_stats.served.size));
  }
  auto block_ratio() const { return block_time / simulation_time; }
};

struct Stats {
  LostServedStats total;
  std::map<TrafficClassId, TrafficClassStats> by_traffic_class;
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
