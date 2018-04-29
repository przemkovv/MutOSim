

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

namespace rng = ranges;
namespace ranges
{
inline namespace v3
{
template <>
struct difference_type<::Count> {
  using type = count_t;
};
} // namespace v3
} // namespace ranges

namespace Model
{
//----------------------------------------------------------------------

//----------------------------------------------------------------------
std::vector<StreamProperties>
convert_to_overflowing_streams(
    const std::vector<std::vector<RequestStream>> &request_streams_per_group)
{
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
  auto inv_sum = rng::accumulate(
                     streams | rng::view::transform([](const auto &rs) {
                       return get_mean(rs) * get_tc(rs).size;
                     }),
                     WeightF{0})
                     .invert();

  // Formula 3.19
  auto peakness = rng::accumulate(
      streams | rng::view::transform([inv_sum](const auto &rs) {
        return get_variance(rs) * (get_tc(rs).size * inv_sum);
      }),
      Peakness{0});

  return peakness;
}

//----------------------------------------------------------------------
Variance
compute_riordan_variance(
    MeanIntensity mean, Intensity intensity, CapacityF fictional_capacity, Size tc_size)
{
  return Variance{get(mean) * (get(intensity) / (get(fictional_capacity) / get(tc_size) +
                                                 1 - get(intensity) + get(mean)) +
                               1 - get(mean))};
}

//----------------------------------------------------------------------
CapacityF
compute_fictional_capacity(
    const std::vector<RequestStream> &request_streams, Capacity V, TrafficClassId tc_id)
{
  return V - rng::accumulate(
                 request_streams | rng::view::filter([tc_id](const auto &rs) {
                   return rs.tc.id != tc_id;
                 }) | rng::view::transform([](const auto &rs) {
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
    RequestStream rs;
    rs.tc = get_tc(stream);
    auto n = V - rs.tc.size + Size{1};
    rs.blocking_probability =
        rng::accumulate(distribution | rng::view::drop(size_t(n)), Probability{0});

    rs.intensity = get_intensity(stream);
    rs.mean = rs.intensity * rs.blocking_probability;
    rs.mean_request_number =
        MeanRequestNumber{rs.intensity * rs.blocking_probability.opposite()};

    request_streams.emplace_back(rs);
  }

  for (auto &rs : request_streams) {
    rs.fictional_capacity = compute_fictional_capacity(request_streams, V, rs.tc.id);

    rs.variance = compute_riordan_variance(
        rs.mean, rs.intensity, rs.fictional_capacity, rs.tc.size);

    rs.peakness = rs.variance / rs.mean;
  }

  return request_streams;
}

//----------------------------------------------------------------------
Count
combinatorial_arrangement_number(Capacity x, Count resources_number, Capacity f)
{
  Count upper_limit{get(x) / (get(f) + 1)};
  auto sum = rng::accumulate(
      rng::view::closed_iota(Count{0}, upper_limit) |
          rng::view::transform([&](Count iota) {
            return Count{
                (1 - 2 * (get(iota) % 2)) *
                Math::n_over_k(get(resources_number), get(iota)) *
                Math::n_over_k(
                    get(x) + get(resources_number) - get(iota) * (get(f) + 1) - 1,
                    get(resources_number) - 1)};
          }),
      Count{0});

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
