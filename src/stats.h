#pragma once

#include "types.h"

#include <unordered_map>
#include <fmt/format.h>

struct LoadStats {
  Size count;
  Size size;
};

struct BlockStats {
  Duration block_time = 0;
  Time start_of_block = 0;
};

struct LostServedStats {
  LoadStats lost;
  LoadStats served;
};

struct Stats {
  LostServedStats total;
  Duration block_time;
  Duration simulation_time;

  std::unordered_map<Uuid, LostServedStats> by_source;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Stats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LoadStats &load_stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const std::unordered_map<Uuid, LoadStats> &served_by_source);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LostServedStats &stats);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const std::unordered_map<Uuid, LostServedStats> &lost_served_stats);
