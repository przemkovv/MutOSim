#pragma once

#include "types.h"

#include <fmt/format.h>
#include <unordered_map>

struct LoadStats {
  Count count;
  Size size;
};

struct BlockStats {
  Duration block_time{0};
  Time start_of_block{0};
  bool is_blocked = false;
};

struct LostServedStats {
  LoadStats lost;
  LoadStats served;
};

struct TrafficClassStats {
  LostServedStats lost_served_stats;
  Duration block_time;
  Duration simulation_time;
};

struct Stats {
  LostServedStats total;
  // Duration block_time;
  // Duration simulation_time;

  std::unordered_map<SourceId, TrafficClassStats> by_source;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Stats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const TrafficClassStats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LoadStats &load_stats);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const std::unordered_map<SourceId, LoadStats> &served_by_source);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LostServedStats &stats);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const std::unordered_map<SourceId, LostServedStats> &lost_served_stats);
