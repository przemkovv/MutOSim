
#include "stats.h"

#include "math.h"
#include <fmt/ostream.h>

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LoadStats &load_stats)
{
  f.writer().write("{} ({}u)", load_stats.count, load_stats.size);
}
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LostServedStats &stats)
{
  auto loss_ratio = Math::ratio_to_sum<double>(ts::get(stats.lost.count),
                                               ts::get(stats.served.count));
  auto loss_ratio_size = Math::ratio_to_sum<double>(ts::get(stats.lost.size),
                                                    ts::get(stats.served.size));

  f.writer().write("served/lost: {} / {}. P_loss: {} ({})", stats.served,
                   stats.lost, loss_ratio, loss_ratio_size);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Stats &stats)
{
  f.writer().write("{}, P_block: {}", stats.total,
                   stats.block_time / stats.simulation_time);
}

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const std::unordered_map<SourceId, LostServedStats> &lost_served_stats)
{
  for (auto & [ source_id, stats ] : lost_served_stats) {
    f.writer().write("source_id={}: {}", source_id, stats);
  }
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const std::unordered_map<SourceId, LoadStats> &served_by_source)
{
  for (auto & [ source_id, stats ] : served_by_source) {
    f.writer().write("source_id={}: {}", source_id, stats);
  }
}
