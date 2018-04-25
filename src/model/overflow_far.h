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
};

std::vector<double> KaufmanRobertsDistribution(
    const std::vector<TrafficClass> &traffic_classes, Capacity V, Capacity n);

std::vector<RequestStream>
KaufmanRobertsBlockingProbability(std::vector<TrafficClass> &traffic_classes, Capacity V);

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
    if (s.size() > 12) {
      f.writer().write("\n");
    }
    f.writer().write(s);
  }
  f.writer().write("]\n");
}

} // namespace std
