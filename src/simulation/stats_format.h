
#pragma once
#include "math_utils.h"
#include "stats.h"
#include "types/hash.h"
#include "types/types_format.h"

#include <fmt/format.h>

namespace Simulation {
} // namespace Simulation

template <>
struct fmt::formatter<Simulation::Stats>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::Stats &stats, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", stats.total);
  }
};

template <>
struct fmt::formatter<Simulation::LoadStats>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::LoadStats &load_stats, FormatContext &ctx) const
  {
    return fmt::format_to(
        ctx.out(),
        "{:>10} ({:>10}u)",
        ts::get(load_stats.count),
        ts::get(load_stats.size));
  }
};

template <>
struct fmt::formatter<Simulation::LostServedStats>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const Simulation::LostServedStats &stats, FormatContext &ctx) const
  {
    auto loss_ratio = Math::ratio_to_sum<double>(
        static_cast<double>(ts::get(stats.lost.count)),
        ts::get(stats.served.count));
    auto loss_ratio_size = Math::ratio_to_sum<double>(
        static_cast<double>(ts::get(stats.lost.size)),
        ts::get(stats.served.size));

    return fmt::format_to(
        ctx.out(),
        "served/lost: {} / {}. P_loss: {:<10} ({:<10})",
        stats.served,
        stats.lost,
        loss_ratio,
        loss_ratio_size);
  }
};

template <>
struct fmt::formatter<std::map<TrafficClassId, Simulation::LostServedStats>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::map<TrafficClassId, Simulation::LostServedStats>
                    &lost_served_stats,
      FormatContext &ctx) const
  {
    for (auto &[tc_id, stats] : lost_served_stats)
    {
      ctx = fmt::format_to(ctx.out(), "tc={}: {}", tc_id, stats);
    }
    return ctx;
  }
};
template <>
struct fmt::formatter<
    std::unordered_map<TrafficClassId, Simulation::LostServedStats>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::unordered_map<TrafficClassId, Simulation::LostServedStats>
                    &lost_served_stats,
      FormatContext &ctx) const
  {
    for (auto &[tc_id, stats] : lost_served_stats)
    {
      ctx = fmt::format_to(ctx.out(), "tc={}: {}", tc_id, stats);
    }
    return ctx;
  }
};
template <>
struct fmt::formatter<std::map<SourceId, Simulation::LoadStats>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::map<SourceId, Simulation::LoadStats> &served_by_traffic_class,
      FormatContext                                   &ctx) const
  {
    for (auto &[source_id, stats] : served_by_traffic_class)
    {
      ctx = fmt::format_to(ctx.out(), "source_id={}: {}", source_id, stats);
    }
    return ctx;
  }
};
template <>
struct fmt::formatter<std::unordered_map<SourceId, Simulation::LoadStats>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const std::unordered_map<SourceId, Simulation::LoadStats>
                    &served_by_traffic_class,
      FormatContext &ctx) const
  {
    for (auto &[source_id, stats] : served_by_traffic_class)
    {
      ctx = fmt::format_to(ctx.out(), "source_id={}: {}", source_id, stats);
    }
    return ctx;
  }
};

template <>
struct fmt::formatter<Simulation::TrafficClassStats>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(const Simulation::TrafficClassStats &stats, FormatContext &ctx) const
  {
    auto p_block = stats.block_time / stats.simulation_time;
    return fmt::format_to(
        ctx.out(),
        "{}, P_block = {:<10} ({:<10})",
        stats.lost_served_stats,
        p_block,
        std::log10(p_block));
  }
};
