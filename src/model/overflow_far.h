#pragma once

#include "traffic_class.h"
#include "types.h"

#include <fmt/ostream.h>
#include <vector>

namespace Model
{
struct RequestStream {
  TrafficClass tc;
  double blocking_probability; // [E_c]_{V_s}
  Intensity intensity;
  double mean;                // R_{c,s}
  double mean_request_number; // Y_{c,s}
  double fictional_capacity;
  double variance_sq; // sigma^2_{c,s}
  double peakness;    // Z_c
};

struct OverflowingRequestStream {
  double variance_sq = 0.0;
  double mean = 0.0;
  double peakness = 0.0;
  TrafficClass tc{};
};

std::vector<double>
KaufmanRobertsDistribution(const std::vector<TrafficClass> &traffic_classes, Capacity V);

std::vector<RequestStream>
KaufmanRobertsBlockingProbability(std::vector<TrafficClass> &traffic_classes, Capacity V);
std::vector<double> KaufmanRobertsDistribution(
    const std::vector<OverflowingRequestStream> &streams_properties,
    Capacity V,
    double peakness);
std::vector<RequestStream> KaufmanRobertsBlockingProbability(
    std::vector<OverflowingRequestStream> &request_streams_properties,
    Capacity V,
    double peakness);

std::vector<OverflowingRequestStream> convert_to_overflowing_streams(
    const std::vector<std::vector<RequestStream>> &request_streams_per_group);

double compute_collective_peakness(
    const std::vector<OverflowingRequestStream> &overflowing_streams);
//----------------------------------------------------------------------

Count combinatorial_arrangement_number(Size x, Count resources_number, Capacity f);

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
                const char *&format_str,
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
