#pragma once

#include "traffic_class.h"
#include "types.h"

#include <boost/variant/variant_fwd.hpp>
#include <fmt/ostream.h>

namespace Model
{
struct RequestStream {
  TrafficClass tc;
  Probability blocking_probability{0}; // [E_c]_{V_s}
  Intensity intensity{0};
  MeanIntensity mean{0};                    // R_{c,s}
  MeanRequestNumber mean_request_number{0}; // Y_{c,s}
  CapacityF fictional_capacity{0};
  VarianceSq variance_sq{0}; // sigma^2_{c,s}
  Peakness peakness{0};      // Z_c
};

struct OverflowingRequestStream {
  TrafficClass tc{};
  MeanIntensity mean{0};
  VarianceSq variance_sq{0};
  Peakness peakness{0};

  OverflowingRequestStream &operator+=(const RequestStream &rs);
  OverflowingRequestStream(const RequestStream &rs);
  OverflowingRequestStream(const OverflowingRequestStream &rs) = default;
};

OverflowingRequestStream operator+(
    OverflowingRequestStream overflowing_request_stream,
    const RequestStream &request_stream);

using StreamProperties = boost::variant<TrafficClass, OverflowingRequestStream>;

const TrafficClass &get_tc(const StreamProperties &v);
Intensity get_intensity(const StreamProperties &v);
VarianceSq get_variance_sq(const StreamProperties &v);
MeanIntensity get_mean(const StreamProperties &v);

//----------------------------------------------------------------------
void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const RequestStream &request_stream);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const OverflowingRequestStream &rs);

} // namespace Model

namespace boost
{
void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const Model::StreamProperties &stream_properties);
} // namespace boost

namespace std
{
template <typename T>
void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const std::vector<T> &vec)
{
  f.writer().write("S({}) [", std::size(vec));
  for (const auto &x : vec) {
    auto s = fmt::format("{}, ", x);
    if (s.size() > 16) {
      f.writer().write("\n");
    }
    f.writer().write(s);
  }
  f.writer().write("]\n");
}

} // namespace std

