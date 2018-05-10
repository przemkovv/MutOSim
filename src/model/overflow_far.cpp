

#include "overflow_far.h"

#include "logger.h"
#include "math_utils.h"

#include <iterator>
#include <map>
#include <range/v3/action/transform.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
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
template <>
struct difference_type<::Capacity> {
  using type = count_t;
};
} // namespace v3
} // namespace ranges

namespace Model
{
//----------------------------------------------------------------------

//----------------------------------------------------------------------
std::vector<IncomingRequestStream>
convert_to_incoming_streams(
    const std::vector<std::vector<OutgoingRequestStream>> &out_request_streams_per_group)
{
  // Formulas 3.17 and 3.18
  std::map<TrafficClassId, IncomingRequestStream> incoming_request_streams;
  for (const auto &out_request_streams : out_request_streams_per_group) {
    rng::for_each(out_request_streams, [&](const OutgoingRequestStream &out_rs) {
      auto [stream_it, inserted] =
          incoming_request_streams.emplace(out_rs.tc.id, IncomingRequestStream{});
      stream_it->second += out_rs;
    });
  }
  return incoming_request_streams | rng::view::values;
}

//----------------------------------------------------------------------
Peakness
compute_collective_peakness(const std::vector<IncomingRequestStream> &in_request_streams)
{
  // Formula 3.20
  auto inv_sum = rng::accumulate(
                     in_request_streams | rng::view::transform([](const auto &rs) {
                       return rs.mean * rs.tc.size;
                     }),
                     WeightF{0})
                     .invert();

  // Formula 3.19
  auto peakness = rng::accumulate(
      in_request_streams | rng::view::transform([inv_sum](const auto &rs) {
        return rs.variance * (rs.tc.size * inv_sum);
      }),
      Peakness{0});

  return peakness;
}

//----------------------------------------------------------------------
Variance
compute_riordan_variance(
    MeanIntensity mean, Intensity intensity, CapacityF fictional_capacity, SizeF tc_size)
{
  println(
      "Compute riordan variance: mean={}, intensity={}, finctional_capacity={}, "
      "tc_size={}",
      mean,
      intensity,
      fictional_capacity,
      tc_size);
  return Variance{get(mean) * (get(intensity) / (get(fictional_capacity) / get(tc_size) +
                                                 1 - get(intensity) + get(mean)) +
                               1 - get(mean))};
}

//----------------------------------------------------------------------
// Serviced traffic fit criterion (Formulas 3.8 and 3.9)
// TODO(PW): implement criterion based on blocking probability fit (Formula 3.10)
CapacityF
compute_fictional_capacity(
    const std::vector<OutgoingRequestStream> &out_request_streams,
    Capacity V,
    TrafficClassId tc_id,
    Peakness size_rescale)
{
  println("Compute fictional capacity: V={}, rescale={}", V, size_rescale);
  return V - rng::accumulate(
                 out_request_streams | rng::view::filter([tc_id](const auto &rs) {
                   return rs.tc.id != tc_id;
                 }) | rng::view::transform([&](const auto &rs) {
                   return rs.mean_request_number * (rs.tc.size * size_rescale);
                 }),
                 CapacityF{0});
}

//----------------------------------------------------------------------
Probabilities
KaufmanRobertsDistribution(
    const std::vector<IncomingRequestStream> &in_request_streams,
    Capacity V,
    Peakness size_rescale)
{
  std::vector<Probability> state(size_t(V) + 1);
  state[0] = Probability{1};

  rng::for_each(rng::view::closed_iota(Capacity{1}, V), [&](Capacity n) {
    for (const auto &in_stream : in_request_streams) {
      auto tc_size = in_stream.tc.size * size_rescale;

      auto previous_state = Capacity{n - tc_size};
      if (previous_state >= Capacity{0}) {
        auto intensity = in_stream.intensity;
        state[size_t(n)] += intensity * tc_size * state[size_t(previous_state)];
      }
    }
    state[size_t(n)] /= n;
    // Math::normalizeN(state, size_t(n));
  });
  Math::normalize(state);
  return state;
} // namespace Model

//----------------------------------------------------------------------
std::vector<OutgoingRequestStream>
KaufmanRobertsBlockingProbability(
    std::vector<IncomingRequestStream> &in_request_streams,
    Capacity V,
    Peakness peakness,
    bool fixed_capacity)
{
  Peakness size_rescale;
  if (fixed_capacity) {
    size_rescale = peakness;
  } else {
    size_rescale = Peakness{1};
    V = Capacity{V / peakness};
  }
  auto distribution = KaufmanRobertsDistribution(in_request_streams, V, size_rescale);

  std::vector<OutgoingRequestStream> out_request_streams;
  for (const auto &in_rs : in_request_streams) {
    OutgoingRequestStream out_rs;
    out_rs.tc = in_rs.tc;
    CapacityF n{V - out_rs.tc.size * size_rescale + Size{1}};
    Capacity n_int{n};
    out_rs.blocking_probability =
        rng::accumulate(distribution | rng::view::drop(size_t(n_int)), Probability{0});

    out_rs.intensity = in_rs.intensity;
    out_rs.mean = out_rs.intensity * out_rs.blocking_probability;
    out_rs.mean_request_number =
        MeanRequestNumber{out_rs.intensity * out_rs.blocking_probability.opposite()};

    out_request_streams.emplace_back(out_rs);
  }

  for (auto &rs : out_request_streams) {
    rs.fictional_capacity =
        compute_fictional_capacity(out_request_streams, V, rs.tc.id, size_rescale);
    println(
        "Compute fictional capacity: V_f={}, tc_id={}", rs.fictional_capacity, rs.tc.id);

    rs.variance = compute_riordan_variance(
        rs.mean, rs.intensity, rs.fictional_capacity, rs.tc.size * size_rescale);
    println("Variance: sigma={}", rs.variance);

    rs.peakness = rs.variance / rs.mean;
    println("Peakedness: Z={}", rs.peakness);
  }

  return out_request_streams;
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
