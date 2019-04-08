
#pragma once

#include "resource.h"

namespace fmt {
template <typename C>
struct formatter<Model::ResourceComponent<C>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Model::ResourceComponent<C> &component, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "(v_s={}, f_s={})", component.number, component.v);
  }
};

template <typename C>
struct formatter<Model::Resource<C>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Model::Resource<C> &resource, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "V={}, components: {}", resource.V(), resource.components);
  }
};
} // namespace fmt
