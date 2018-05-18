#pragma once

#include "traffic_class.h"
#include "types.h"

#include <fmt/ostream.h>
#include <vector>

namespace Model
{
struct OutgoingRequestStream {
  TrafficClass tc;
  Probability blocking_probability{0}; // [E_c]_{V_s}
  Intensity intensity{0};

  MeanIntensity mean{0};                    // R_{c,s}
  MeanRequestNumber mean_request_number{0}; // Y_{c,s}

  CapacityF fictitous_capacity{0};
  Variance variance{0};     // sigma^2_{c,s}
  Peakedness peakedness{0}; // Z_c

  OutgoingRequestStream() = default;
  OutgoingRequestStream(
      const TrafficClass &tc,
      const Probability &blocking_probability,
      const Intensity &intensity);
};

struct IncomingRequestStream {
  TrafficClass tc{};
  MeanIntensity mean{0};
  Variance variance{0};
  Peakedness peakedness{0};
  Intensity intensity{0};

  IncomingRequestStream &operator+=(const OutgoingRequestStream &rs);
  IncomingRequestStream &operator+=(const TrafficClass &source_tc);
  IncomingRequestStream &operator+=(const IncomingRequestStream &rs);
  IncomingRequestStream &operator=(const IncomingRequestStream &rs) = default;
  IncomingRequestStream(const OutgoingRequestStream &rs);
  IncomingRequestStream(const TrafficClass &source_tc);
  IncomingRequestStream(const IncomingRequestStream &rs);
  IncomingRequestStream() = default;
};

using OutgoingRequestStreams = std::vector<OutgoingRequestStream>;
using IncomingRequestStreams = std::vector<IncomingRequestStream>;

//----------------------------------------------------------------------
void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const OutgoingRequestStream &rs);

void format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const IncomingRequestStream &rs);

} // namespace Model

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
