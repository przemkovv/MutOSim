#include "stats.h"

#include "math.h"
#include <fmt/ostream.h>

LoadStats operator+(const LoadStats &s1, const LoadStats &s2)
{
  return {s1.count + s2.count, s1.size + s2.size};
}

LoadStats &operator+=(LoadStats &s1, const LoadStats &s2)
{
  s1.count += s2.count;
  s1.size += s2.size;
  return s1;
}

LostServedStats operator+(const LostServedStats &s1, const LostServedStats &s2)
{
  return {s1.lost + s2.lost, s1.served + s2.served};
}

LostServedStats &operator+=(LostServedStats &s1, const LostServedStats &s2)
{
  s1.lost += s2.lost;
  s1.served += s2.served;
  return s1;
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LoadStats &load_stats)
{
  f.writer().write("{:>10n} ({:>10n}u)", ts::get(load_stats.count),
                   ts::get(load_stats.size));
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LostServedStats &stats)
{
  auto loss_ratio =
      Math::ratio_to_sum<double>(ts::get(stats.lost.count), ts::get(stats.served.count));
  auto loss_ratio_size =
      Math::ratio_to_sum<double>(ts::get(stats.lost.size), ts::get(stats.served.size));

  f.writer().write("served/lost: {} / {}. P_loss: {:<10} ({:<10})", stats.served,
                   stats.lost, loss_ratio, loss_ratio_size);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Stats &stats)
{
  f.writer().write("{}", stats.total);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const TrafficClassStats &stats)
{
  auto p_block = stats.block_time / stats.simulation_time;
  f.writer().write("{}, P_block = {:<10} ({:<10})", stats.lost_served_stats,
                   p_block, std::log10(p_block));
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const std::map<TrafficClassId, LostServedStats> &lost_served_stats)
{
  for (auto &[tc_id, stats] : lost_served_stats) {
    f.writer().write("source_id={}: {}", tc_id, stats);
  }
}

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const std::unordered_map<TrafficClassId, LostServedStats> &lost_served_stats)
{
  for (auto &[tc_id, stats] : lost_served_stats) {
    f.writer().write("source_id={}: {}", tc_id, stats);
  }
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const std::unordered_map<SourceId, LoadStats> &served_by_traffic_class)
{
  for (auto &[tc_id, stats] : served_by_traffic_class) {
    f.writer().write("source_id={}: {}", tc_id, stats);
  }
}
