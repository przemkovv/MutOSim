
#include "overflow_far.h"
#include "logger.h"
#include "math_utils.h"
#include <fmt/ostream.h>

namespace Model
{
std::vector<double> KaufmanRobertsDistribution(
    const std::vector<TrafficClass> &traffic_classes, Capacity V, Capacity n)
{
  std::vector<double> state(get(V) + 1);
  state[0] = 1;

  println("[KR] V={}, n={}", V, n);
  for (auto v = Capacity{1}; v < V + Capacity(1); ++v) {
    for (const auto &tc : traffic_classes) {
      auto previous_state = v - tc.size;
      if (previous_state >= Capacity{0}) {
        auto intensity = tc.source_intensity / tc.serve_intensity;
        state[get(v)] += get(intensity) * get(tc.size) * state[get(previous_state)];
      }
    }
    state[get(v)] /= get(v);
  }
  Math::normalize(state);

  print("{}", state);
  return state;
}

std::vector<double>
KaufmanRobertsDistribution(const std::vector<RequestStreamProperties> &streams_properties,
                           Capacity V,
                           double peakness)
{
  V = Capacity(get(V) / peakness);
  std::vector<double> state(get(V) + 1);
  state[0] = 1;

  println("[KR] V={}, Z={}", V, peakness);
  for (auto v = Capacity{1}; v < V + Capacity(1); ++v) {
    for (const auto &properties : streams_properties) {
      auto previous_state = v - properties.tc.size;
      if (previous_state >= Capacity{0}) {
        auto intensity = properties.mean / properties.peakness;
        state[get(v)] += intensity * get(properties.tc.size) * state[get(previous_state)];
      }
    }
    state[get(v)] /= get(v);
  }
  Math::normalize(state);

  print("{}", state);
  return state;
}

std::vector<RequestStream>
KaufmanRobertsBlockingProbability(std::vector<TrafficClass> &traffic_classes, Capacity V)
{
  auto distribution = KaufmanRobertsDistribution(traffic_classes, V, V);

  std::vector<RequestStream> request_streams;
  for (const auto &tc : traffic_classes) {
    auto n = V - tc.size + Size{1};
    double blocking_probability =
        std::accumulate(next(begin(distribution), get(n)), end(distribution), 0.0);

    auto intensity = tc.source_intensity / tc.serve_intensity;
    double mean = get(intensity) * blocking_probability;
    double mean_request_number = get(intensity) * (1 - blocking_probability);

    request_streams.emplace_back(
        RequestStream{tc, blocking_probability, intensity, mean, mean_request_number});
  }

  for (auto &rs : request_streams) {
    auto fictional_capacity =
        get(V) - std::accumulate(begin(request_streams), end(request_streams), 0.0,
                                 [&rs](auto v, const auto &request_stream) {
                                   if (request_stream.tc.id == rs.tc.id) {
                                     return v;
                                   } else {
                                     return v + request_stream.mean_request_number *
                                                    get(request_stream.tc.size);
                                   }
                                 });

    rs.variance_sq =
        rs.mean * (get(rs.intensity) / (fictional_capacity / get(rs.tc.size) + 1 -
                                        get(rs.intensity) + rs.mean) +
                   1 - rs.mean);

    rs.peakness = rs.variance_sq / rs.mean;
  }

  return request_streams;
}

std::vector<RequestStream> KaufmanRobertsBlockingProbability(
    std::vector<RequestStreamProperties> &request_streams_properties,
    Capacity V,
    double peakness)
{
  auto distribution = KaufmanRobertsDistribution(request_streams_properties, V, peakness);

  V = Capacity( get(V)/peakness );

  std::vector<RequestStream> request_streams;
  for (const auto &properties : request_streams_properties) {
    auto n = V - properties.tc.size + Size{1};
    double blocking_probability =
        std::accumulate(next(begin(distribution), get(n)), end(distribution), 0.0);

    auto intensity = Intensity{properties.mean / properties.peakness};
    double mean = get(intensity) * blocking_probability;
    double mean_request_number = get(intensity) * (1 - blocking_probability);

    request_streams.emplace_back(
        RequestStream{properties.tc, blocking_probability, intensity, mean, mean_request_number});
  }

  for (auto &rs : request_streams) {
    auto fictional_capacity =
        get(V) - std::accumulate(begin(request_streams), end(request_streams), 0.0,
                                 [&rs](auto v, const auto &request_stream) {
                                   if (request_stream.tc.id == rs.tc.id) {
                                     return v;
                                   } else {
                                     return v + request_stream.mean_request_number *
                                                    get(request_stream.tc.size);
                                   }
                                 });

    rs.variance_sq =
        rs.mean * (get(rs.intensity) / (fictional_capacity / get(rs.tc.size) + 1 -
                                        get(rs.intensity) + rs.mean) +
                   1 - rs.mean);

    rs.peakness = rs.variance_sq / rs.mean;
  }

  return request_streams;
}

//----------------------------------------------------------------------

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const RequestStream &rs)
{
  f.writer().write("[RequestStream] {} Blocking probability {}, R={}, sigma^2={}, Z={}",
                   rs.tc, rs.blocking_probability, rs.mean, rs.variance_sq, rs.peakness);
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const RequestStreamProperties &rs)
{
  f.writer().write("[RequestStreamProperties] {} R={}, sigma^2={}, Z={}", rs.tc, rs.mean,
                   rs.variance_sq, rs.peakness);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

} // namespace Model
