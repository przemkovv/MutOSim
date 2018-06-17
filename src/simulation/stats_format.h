
#pragma once
#include "stats.h"

#include <fmt/format.h>

namespace Simulation
{
} // namespace Simulation

namespace fmt
{
template <>
struct formatter<Simulation::Stats> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::Stats &stats, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", stats.total);
  }
};

template <>
struct formatter<Simulation::LoadStats> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::LoadStats &load_stats, FormatContext &ctx)
  {
    return format_to(
        ctx.begin(),
        "{:>10n} ({:>10n}u)",
        ts::get(load_stats.count),
        ts::get(load_stats.size));
  }
};

template <>
struct formatter<Simulation::LostServedStats> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::LostServedStats &stats, FormatContext &ctx)
  {
    auto loss_ratio = Math::ratio_to_sum<double>(
        ts::get(stats.lost.count), ts::get(stats.served.count));
    auto loss_ratio_size =
        Math::ratio_to_sum<double>(ts::get(stats.lost.size), ts::get(stats.served.size));

    return format_to(
        ctx.begin(),
        "served/lost: {} / {}. P_loss: {:<10} ({:<10})",
        stats.served,
        stats.lost,
        loss_ratio,
        loss_ratio_size);
  }
};

template <>
struct formatter<std::map<TrafficClassId, Simulation::LostServedStats>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::map<TrafficClassId, Simulation::LostServedStats> &lost_served_stats,
      FormatContext &ctx)
  {
    for (auto &[tc_id, stats] : lost_served_stats) {
      ctx = format_to(ctx.begin(), "tc={}: {}", tc_id, stats);
    }
  }
};
template <>
struct formatter<std::unordered_map<TrafficClassId, Simulation::LostServedStats>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::unordered_map<TrafficClassId, Simulation::LostServedStats>
          &lost_served_stats,
      FormatContext &ctx)
  {
    for (auto &[tc_id, stats] : lost_served_stats) {
      ctx = format_to(ctx.begin(), "tc={}: {}", tc_id, stats);
    }
  }
};
template <>
struct formatter<std::map<SourceId, Simulation::LoadStats>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::map<SourceId, Simulation::LoadStats> &served_by_traffic_class,
      FormatContext &ctx)
  {
    for (auto &[source_id, stats] : served_by_traffic_class) {
      ctx = format_to(ctx.begin(), "source_id={}: {}", source_id, stats);
    }
  }
};
template <>
struct formatter<std::unordered_map<SourceId, Simulation::LoadStats>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::unordered_map<SourceId, Simulation::LoadStats> &served_by_traffic_class,
      FormatContext &ctx)
  {
    for (auto &[source_id, stats] : served_by_traffic_class) {
      ctx = format_to(ctx.begin(), "source_id={}: {}", source_id, stats);
    }
  }
};

template <>
struct formatter<Simulation::TrafficClassStats> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::TrafficClassStats &stats, FormatContext &ctx)
  {
    auto p_block = stats.block_time / stats.simulation_time;
    return format_to(
        ctx.begin(),
        "{}, P_block = {:<10} ({:<10})",
        stats.lost_served_stats,
        p_block,
        std::log10(p_block));
  }
};
} // namespace fmt
