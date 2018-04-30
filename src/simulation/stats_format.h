
#include "stats.h"

#include <fmt/format.h>

void
format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const Stats &stats);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const TrafficClassStats &stats);

void format_arg(
    fmt::BasicFormatter<char> &f, const char *&format_str, const LoadStats &load_stats);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const std::unordered_map<SourceId, LoadStats> &served_by_traffic_class);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const LostServedStats &stats);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const std::map<TrafficClassId, LostServedStats> &lost_served_stats);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const std::unordered_map<TrafficClassId, LostServedStats> &lost_served_stats);

