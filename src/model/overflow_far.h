#pragma once

#include "traffic_class.h"
#include "types.h"

#include <fmt/ostream.h>
#include <vector>

namespace Model
{
struct RequestStream {
  TrafficClass tc;
  Probability blocking_probability{0}; // [E_c]_{V_s}
  Intensity intensity{0};
  stat_t mean{0};                // R_{c,s}
  stat_t mean_request_number{0}; // Y_{c,s}
  stat_t fictional_capacity{0};
  stat_t variance_sq{0}; // sigma^2_{c,s}
  stat_t peakness{0};    // Z_c
};

struct OverflowingRequestStream {
  stat_t variance_sq = 0.0L;
  stat_t mean = 0.0L;
  stat_t peakness = 0.0L;
  TrafficClass tc{};
};

std::vector<Probability>
KaufmanRobertsDistribution(const std::vector<TrafficClass> &traffic_classes, Capacity V);

std::vector<RequestStream>
KaufmanRobertsBlockingProbability(std::vector<TrafficClass> &traffic_classes, Capacity V);

std::vector<Probability> KaufmanRobertsDistribution(
    const std::vector<OverflowingRequestStream> &streams_properties,
    Capacity V,
    stat_t peakness);
std::vector<RequestStream> KaufmanRobertsBlockingProbability(
    std::vector<OverflowingRequestStream> &request_streams_properties,
    Capacity V,
    stat_t peakness);

std::vector<OverflowingRequestStream> convert_to_overflowing_streams(
    const std::vector<std::vector<RequestStream>> &request_streams_per_group);

stat_t compute_collective_peakness(
    const std::vector<OverflowingRequestStream> &overflowing_streams);
//----------------------------------------------------------------------

Count combinatorial_arrangement_number(count_t x, Count resources_number, Capacity f);

probability_t transition_probability(
    Capacity n, Capacity V, Count resources_number, Capacity f, Size t);

//----------------------------------------------------------------------
void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const RequestStream &request_stream);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const OverflowingRequestStream &rs);

} // namespace Model

namespace std
{
template <typename T>
void format_arg(fmt::BasicFormatter<char> &f,
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
