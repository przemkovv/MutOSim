#pragma once

#include "cli_options.h"

#include <fmt/format.h>
#include <string_view>

namespace boost::program_options {
class options_description;
}

namespace fmt {
template <>
struct formatter<Mode> : formatter<std::string_view>
{
  template <typename FormatContext>
  auto format(const Mode &t, FormatContext &ctx)
  {
    return formatter<std::string_view>::format(
        [](Mode value) {
          switch (value)
          {
            case Mode::Analytic:
              return "simulation";
            case Mode::Simulation:
              return "analytic";
            case Mode::Test:
              return "test";
          }
        }(t),
        ctx);
  }
};
template <>
struct formatter<Model::AnalyticModel> : formatter<std::string_view>
{
  template <typename FormatContext>
  auto format(const Model::AnalyticModel &t, FormatContext &ctx)
  {
    return formatter<std::string_view>::format(
        [](Model::AnalyticModel value) {
          switch (value)
          {
            case Model::AnalyticModel::KaufmanRobertsFixedCapacity:
              return "KRFixedCapacity";
            case Model::AnalyticModel::KaufmanRobertsFixedReqSize:
              return "KRFixedReqSize";
          }
        }(t),
        ctx);
  }
};
template <>
struct formatter<boost::program_options::options_description>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const boost::program_options::options_description &t, FormatContext &ctx)
  {
    std::stringstream str;
    t.print(str);
    return format_to(ctx.begin(), "{}", str.str());
  }
};
} // namespace fmt
