#pragma once

#include "traffic_class.h"
#include "types.h"

#include <fmt/ostream.h>
#include <vector>

namespace Model
{
struct Moments {
  MeanIntensity mean{};
  VarianceSq variance_sq{};
  Peakness
  peakness() const
  {
    return variance_sq / mean;
  }
};
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
  VarianceSq variance_sq{0};
  MeanIntensity mean{0};
  Peakness peakness{0};
  TrafficClass tc{};
};

using Probabilities = std::vector<Probability>;

Probabilities
KaufmanRobertsDistribution(const std::vector<TrafficClass> &traffic_classes, Capacity V);
Probabilities
KaufmanRobertsDistribution(const std::vector<OverflowingRequestStream> &request_streams,
                           Capacity V);

std::vector<RequestStream>
KaufmanRobertsBlockingProbability(std::vector<TrafficClass> &traffic_classes, Capacity V);

std::vector<RequestStream>
KaufmanRobertsBlockingProbability(
    std::vector<OverflowingRequestStream> &request_streams_properties,
    Capacity V,
    Peakness peakness);

//----------------------------------------------------------------------
std::vector<OverflowingRequestStream>
convert_to_overflowing_streams(
    const std::vector<std::vector<RequestStream>> &request_streams_per_group);

//----------------------------------------------------------------------
Peakness
compute_collective_peakness(
    const std::vector<OverflowingRequestStream> &overflowing_streams);

VarianceSq
compute_riordan_variance(MeanIntensity mean,
                         Intensity intensity,
                         CapacityF fictional_capacity,
                         Size tc_size);
CapacityF
compute_fictional_capacity(const std::vector<RequestStream> &request_streams,
                           Capacity V,
                           TrafficClassId tc_id);
Count
combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f);

Probability
transition_probability(
    Capacity n, Capacity V, Count resources_number, Capacity f, Size t);

//----------------------------------------------------------------------
void
format_arg(fmt::BasicFormatter<char> &f,
           const char *&format_str,
           const RequestStream &request_stream);

void
format_arg(fmt::BasicFormatter<char> &f,
           const char *&format_str,
           const OverflowingRequestStream &rs);

} // namespace Model

namespace std
{
template <typename T>
void
format_arg(fmt::BasicFormatter<char> &f,
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
