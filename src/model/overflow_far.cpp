

#include "overflow_far.h"

#include "logger.h"
#include "math_utils.h"

#include <boost/variant/variant.hpp>
#include <iterator>
#include <range/v3/action/transform.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/all.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace Model
{
//----------------------------------------------------------------------

//----------------------------------------------------------------------
std::vector<StreamProperties>
convert_to_overflowing_streams(
    const std::vector<std::vector<RequestStream>> &request_streams_per_group)
{
  using namespace ranges;
  // Formulas 3.17 and 3.18
  // NOTE(PW) Does not aggregates over traffic class id
  std::vector<StreamProperties> overflowing_request_streams;
  for (const auto &request_streams : request_streams_per_group) {
    std::transform(
        begin(request_streams),
        end(request_streams),
        std::back_inserter(overflowing_request_streams),
        [](const auto &rs) -> OverflowingRequestStream { return {rs}; });
  }
  return overflowing_request_streams;
}

//----------------------------------------------------------------------
Peakness
compute_collective_peakness(const std::vector<StreamProperties> &streams)
{
  // Formula 3.20
  using namespace ranges;
  auto inv_sum = accumulate(
                     streams | view::transform([](const auto &rs) {
                       return get_mean(rs) * get_tc(rs).size;
                     }),
                     WeightF{0})
                     .invert();

  // Formula 3.19
  auto peakness = accumulate(
      streams | view::transform([inv_sum](const auto &rs) {
        return get_variance_sq(rs) * (get_tc(rs).size * inv_sum);
      }),
      Peakness{0});

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
  using namespace ranges;
  return V - accumulate(
                 request_streams | view::filter([tc_id](const auto &rs) {
                   return rs.tc.id != tc_id;
                 }) | view::transform([](const auto &rs) {
                   return rs.mean_request_number * rs.tc.size;
                 }),
                 CapacityF{0});
}

//----------------------------------------------------------------------
Probabilities
KaufmanRobertsDistribution(const std::vector<StreamProperties> &streams, Capacity V)
{
  std::vector<Probability> state(size_t(V) + 1);
  state[0] = Probability{1};

  println("[KR] V={}", V);
  for (auto n = Capacity{1}; n <= V; ++n) {
    for (const auto &stream : streams) {
      auto tc_size = get_tc(stream).size;
      auto previous_state = n - tc_size;
      if (previous_state >= Capacity{0}) {
        auto intensity = get_intensity(stream);
        state[size_t(n)] += intensity * tc_size * state[size_t(previous_state)];
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
KaufmanRobertsBlockingProbability(std::vector<StreamProperties> &streams, Capacity V)
{
  auto distribution = KaufmanRobertsDistribution(streams, V);

  std::vector<RequestStream> request_streams;
  for (const auto &stream : streams) {
    auto n = V - get_tc(stream).size + Size{1};
    auto blocking_probability = std::accumulate(
        next(begin(distribution), ptrdiff_t(n)), end(distribution), Probability{});

    auto intensity = get_intensity(stream);
    auto mean = intensity * blocking_probability;
    auto mean_request_number =
        MeanRequestNumber{intensity * blocking_probability.opposite()};

    request_streams.emplace_back(RequestStream{
        get_tc(stream), blocking_probability, intensity, mean, mean_request_number});
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

} // namespace Model
