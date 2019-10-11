
#pragma once
#include "engset.h"
#include "pascal.h"
#include "poisson.h"
#include "simulation/world.h"
#include "source_stream.h"

#include <fmt/format.h>

namespace fmt {
template <>
struct formatter<Simulation::SourceStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::SourceStream &source, FormatContext &ctx)
  {
    return format_to(ctx.out(), "[Source {} id={:>2}]", source.name_, source.id);
  }
};

template <>
struct formatter<Simulation::EngsetSourceStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::EngsetSourceStream &source, FormatContext &ctx)
  {
    return format_to(
        ctx.out(),
        "[EngsetSource {} (id={}), active={}/{}, gamma={}, lambda={}]",
        source.name_,
        source.id,
        source.active_sources_,
        source.sources_number_,
        source.tc_.serve_intensity,
        (source.sources_number_ - source.active_sources_) * source.tc_.serve_intensity);
  }
};

template <>
struct formatter<Simulation::PascalSourceStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::PascalSourceStream &source, FormatContext &ctx)
  {
    return format_to(
        ctx.out(),
        "t={} [PascalSource {} (id={}), active={}/{}/{}/{}, gamma={}]",
        source.world_->get_current_time(),
        source.name_,
        source.id,
        source.active_sources_,
        source.sources_number_,
        source.linked_sources_count_,
        source.linked_sources_.size(),
        source.tc_.source_intensity);
  }
};

template <>
struct formatter<Simulation::PoissonSourceStream>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Simulation::PoissonSourceStream &source, FormatContext &ctx)
  {
    return format_to(ctx.out(), "[PoissonSource {} (id={})]", source.name_, source.id);
  }
};

} // namespace fmt
