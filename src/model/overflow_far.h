#pragma once

#include "traffic_class.h"
#include "types.h"

#include <vector>

namespace Model
{
struct RequestStream {
  TrafficClass tc;
  double blocking_probability; // [E_c]_{V_s}
  Intensity intensity;
  double mean;                // R_{c,s}
  double mean_request_number; // Y_{c,s}
  double variance_sq;         // sigma^2_{c,s}
  double peakness;            // Z_c
};

struct RequestStreamProperties {
  double variance_sq = 0.0;
  double mean = 0.0;
  double peakness = 0.0;
  TrafficClass tc{};
};

std::vector<double> KaufmanRobertsDistribution(
    const std::vector<TrafficClass> &traffic_classes, Capacity V, Capacity n);

std::vector<RequestStream>
KaufmanRobertsBlockingProbability(std::vector<TrafficClass> &traffic_classes, Capacity V);
std::vector<double>
KaufmanRobertsDistribution(const std::vector<RequestStreamProperties> &streams_properties,
                           Capacity V,
                           double peakness);
std::vector<RequestStream> KaufmanRobertsBlockingProbability(
    std::vector<RequestStreamProperties> &request_streams_properties,
    Capacity V,
    double peakness);

class OverflowFar
{
  struct PrimaryResource {
    Capacity capacity;
    std::vector<RequestStream> request_streams;
  };

  std::vector<PrimaryResource> primary_resources_;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const RequestStream &request_stream);

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const RequestStreamProperties &rs);
} // namespace Model

namespace std
{
template <typename T>
void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const std::vector<T> &vec)
{
  f.writer().write("[");
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
