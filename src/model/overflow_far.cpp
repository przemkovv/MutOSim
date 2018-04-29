

#include "overflow_far.h"

#include "logger.h"
#include "math_utils.h"

namespace Model
{
//----------------------------------------------------------------------
std::vector<OverflowingRequestStream>
convert_to_overflowing_streams(
    const std::vector<std::vector<RequestStream>> &request_streams_per_group)
{
  // Formulas 3.17 and 3.18
  std::vector<OverflowingRequestStream> overflowing_request_streams;
  for (const auto &request_streams : request_streams_per_group) {
    for (const auto &request_stream : request_streams) {
      auto &overflowing_stream = overflowing_request_streams.emplace_back();
      overflowing_stream.mean += request_stream.mean;
      overflowing_stream.variance_sq += request_stream.variance_sq;
      overflowing_stream.tc = request_stream.tc;
    }
  }
  std::for_each(
      begin(overflowing_request_streams),
      end(overflowing_request_streams),
      [](auto &overflowing_request_stream) {
        overflowing_request_stream.peakness =
            overflowing_request_stream.variance_sq / overflowing_request_stream.mean;
      });
  return overflowing_request_streams;
}

//----------------------------------------------------------------------
Peakness
compute_collective_peakness(
    const std::vector<OverflowingRequestStream> &overflowing_streams)
{
  // Formula 3.20
  auto inv_sum = std::accumulate(
                     begin(overflowing_streams),
                     end(overflowing_streams),
                     WeightF{},
                     [](auto x, const auto &rs) { return x + rs.mean * rs.tc.size; })
                     .invert();

  // Formula 3.19
  auto peakness = std::accumulate(
      begin(overflowing_streams),
      end(overflowing_streams),
      Peakness{},
      [inv_sum](auto x, const auto &rs) {
        return x + rs.variance_sq * (rs.tc.size * inv_sum);
      });

  return peakness;
}

//----------------------------------------------------------------------
VarianceSq
compute_riordan_variance(
    MeanIntensity mean, Intensity intensity, CapacityF fictional_capacity, Size tc_size)
{
  return VarianceSq{get(mean) *
                    (get(intensity) / (get(fictional_capacity) / get(tc_size) + 1 -
                                       get(intensity) + get(mean)) +
                     1 - get(mean))};
}

//----------------------------------------------------------------------
CapacityF
compute_fictional_capacity(
    const std::vector<RequestStream> &request_streams, Capacity V, TrafficClassId tc_id)
{
  return V - std::accumulate(
                 begin(request_streams),
                 end(request_streams),
                 CapacityF{},
                 [&tc_id](auto acc, const auto &request_stream) {
                   if (request_stream.tc.id == tc_id) {
                     return acc;
                   } else {
                     return acc +
                            request_stream.mean_request_number * request_stream.tc.size;
                   }
                 });
}
//----------------------------------------------------------------------
Probabilities
KaufmanRobertsDistribution(const std::vector<TrafficClass> &traffic_classes, Capacity V)
{
  std::vector<Probability> state(size_t(V) + 1);
  state[0] = Probability{1};

  println("[KR] V={}", V);
  for (auto n = Capacity{1}; n <= V; ++n) {
    for (const auto &tc : traffic_classes) {
      auto previous_state = n - tc.size;
      if (previous_state >= Capacity{0}) {
        auto intensity = tc.source_intensity / tc.serve_intensity;
        state[size_t(n)] += intensity * tc.size * state[size_t(previous_state)];
      }
    }
    state[size_t(n)] /= n;
  }
  Math::normalize(state);

  print("{}", state);
  return state;
}

//----------------------------------------------------------------------
Probabilities
KaufmanRobertsDistribution(
    const std::vector<OverflowingRequestStream> &request_streams, Capacity V)
{
  std::vector<Probability> state(size_t(V) + 1);
  state[0] = Probability{1};

  for (auto n = Capacity{1}; n <= V; ++n) {
    for (const auto &properties : request_streams) {
      auto previous_state = n - properties.tc.size;
      if (previous_state >= Capacity{0}) {
        auto intensity = Intensity{properties.mean / properties.peakness};
        state[size_t(n)] +=
            intensity * properties.tc.size * state[size_t(previous_state)];
      }
    }
    state[size_t(n)] /= n;
  }
  Math::normalize(state);

  print("{}", state);
  return state;
}

//----------------------------------------------------------------------
std::vector<RequestStream>
KaufmanRobertsBlockingProbability(std::vector<TrafficClass> &traffic_classes, Capacity V)
{
  auto distribution = KaufmanRobertsDistribution(traffic_classes, V);

  std::vector<RequestStream> request_streams;
  for (const auto &tc : traffic_classes) {
    auto n = V - tc.size + Size{1};
    auto blocking_probability = std::accumulate(
        next(begin(distribution), ptrdiff_t(n)), end(distribution), Probability{});

    auto intensity = tc.source_intensity / tc.serve_intensity;
    auto mean = intensity * blocking_probability;
    auto mean_request_number =
        MeanRequestNumber{intensity * blocking_probability.opposite()};

    request_streams.emplace_back(
        RequestStream{tc, blocking_probability, intensity, mean, mean_request_number});
  }

  for (auto &rs : request_streams) {
    rs.fictional_capacity = compute_fictional_capacity(request_streams, V, rs.tc.id);

    rs.variance_sq = compute_riordan_variance(
        rs.mean, rs.intensity, rs.fictional_capacity, rs.tc.size);

    rs.peakness = rs.variance_sq / rs.mean;
  }

  return request_streams;
}

//----------------------------------------------------------------------
std::vector<RequestStream>
KaufmanRobertsBlockingProbability(
    std::vector<OverflowingRequestStream> &overflowing_streams,
    Capacity V,
    Peakness peakness)
{
  println("[KR] V={}, Z={}, V/Z={}", V, peakness, V / peakness);
  V = Capacity{V / peakness};
  auto distribution = KaufmanRobertsDistribution(overflowing_streams, V);

  std::vector<RequestStream> request_streams;
  for (const auto &overflowing_stream : overflowing_streams) {
    auto n = V - overflowing_stream.tc.size + Size{1};
    auto blocking_probability = std::accumulate(
        next(begin(distribution), ptrdiff_t(n)), end(distribution), Probability{});

    auto intensity = Intensity{overflowing_stream.mean / overflowing_stream.peakness};
    auto mean = intensity * blocking_probability;
    auto mean_request_number =
        MeanRequestNumber{intensity * blocking_probability.opposite()};

    request_streams.emplace_back(RequestStream{overflowing_stream.tc,
                                               blocking_probability,
                                               intensity,
                                               mean,
                                               mean_request_number});
  }

  for (auto &rs : request_streams) {
    rs.fictional_capacity = compute_fictional_capacity(request_streams, V, rs.tc.id);

    rs.variance_sq = compute_riordan_variance(
        rs.mean, rs.intensity, rs.fictional_capacity, rs.tc.size);

    rs.peakness = rs.variance_sq / rs.mean;
  }

  return request_streams;
}
//----------------------------------------------------------------------
Count
combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f)
{
  count_t upper_limit{get(x) / (get(f) + 1)};
  count_t sum = 0;
  for (count_t iota = 0; iota <= upper_limit; ++iota) {
    auto partial_sum = Math::n_over_k(get(resources_number), iota) *
                       Math::n_over_k(
                           get(x) + get(resources_number) - iota * (get(f) + 1) - 1,
                           get(resources_number) - 1);
    sum += (1 - 2 * (0x01 & iota)) * partial_sum;
  }
  return Count{sum};
}
//----------------------------------------------------------------------

Probability
transition_probability(Capacity n, Capacity V, Count resources_number, Capacity f, Size t)
{
  auto nominator =
      combinatorial_arrangement_number(V - n, resources_number, Capacity{get(t) - 1});
  auto denominator = combinatorial_arrangement_number(V - n, resources_number, f);
  return {Probability{1} - Probability{nominator / denominator}};
}

//----------------------------------------------------------------------

void
format_arg(
    fmt::BasicFormatter<char> &f, const char *& /* format_str */, const RequestStream &rs)
{
  f.writer().write(
      "[RequestStream] {} Blocking probability {}, V={}, R={}, sigma^2={}, Z={}, Y={}",
      rs.tc,
      rs.blocking_probability,
      rs.fictional_capacity,
      rs.mean,
      rs.variance_sq,
      rs.peakness,
      rs.mean_request_number);
}

//----------------------------------------------------------------------
void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const OverflowingRequestStream &rs)
{
  f.writer().write(
      "[OverflowingRequestStream] {} R={}, sigma^2={}, Z={}",
      rs.tc,
      rs.mean,
      rs.variance_sq,
      rs.peakness);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

} // namespace Model
