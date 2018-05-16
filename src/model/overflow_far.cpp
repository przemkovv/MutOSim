

#include "overflow_far.h"

#include "calculation.h"
#include "kaufman_roberts.h"
#include "logger.h"
#include "math_utils.h"

#include <iterator>
#include <map>
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
IncomingRequestStreams
convert_to_incoming_streams(
    const std::vector<OutgoingRequestStreams> &out_request_streams_per_group)
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
Peakedness
compute_collective_peakedness(const IncomingRequestStreams &in_request_streams)
{
  // Formula 3.20
  auto inv_sum = rng::accumulate(
                     in_request_streams | rng::view::transform([](const auto &rs) {
                       return rs.mean * rs.tc.size;
                     }),
                     WeightF{0})
                     .invert();

  // Formula 3.19
  auto peakedness = rng::accumulate(
      in_request_streams | rng::view::transform([inv_sum](const auto &rs) {
        return rs.variance * (rs.tc.size * inv_sum);
      }),
      Peakedness{0});

  return peakedness;
}

//----------------------------------------------------------------------
Variance
compute_riordan_variance(
    MeanIntensity mean, Intensity intensity, CapacityF fictional_capacity, SizeF tc_size)
{
  /* clang-format off */
  return Variance{
    get(mean) * (get(intensity) /
                 (
                  (
                   get(fictional_capacity) / get(tc_size)
                  ) + 1 - get(intensity) + get(mean)
                 ) + 1 - get(mean)
                )
  };
  /* clang-format on */
}

//----------------------------------------------------------------------
// Serviced traffic fit criterion (Formulas 3.8 and 3.9)
CapacityF
compute_fictional_capacity_fit_carried_traffic(
    const OutgoingRequestStreams &out_request_streams,
    CapacityF V,
    TrafficClassId tc_id,
    KaufmanRobertsVariant kr_variant)
{
  return V - rng::accumulate(
                 out_request_streams | rng::view::filter([tc_id](const auto &rs) {
                   return rs.tc.id != tc_id;
                 }) | rng::view::transform([&](const auto &rs) {
                   if (kr_variant == KaufmanRobertsVariant::FixedCapacity) {
                     return rs.mean_request_number *
                            (SizeRescale{rs.peakedness} * rs.tc.size);
                   } else {
                     return rs.mean_request_number * rs.tc.size;
                   }
                 }),
                 CapacityF{0});
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// criterion based on blocking probability fit (Formula 3.10)
CapacityF
compute_fictional_capacity_fit_blocking_probability(
    const OutgoingRequestStream &rs, CapacityF V)
{
  count_float_t epsilon{0.000001L};
  count_float_t left_bound{1};
  count_float_t right_bound{get(V)};
  count_float_t current{left_bound};

  while (right_bound - left_bound > epsilon) {
    current = (right_bound + left_bound) * 0.5L;
    auto p = extended_erlang_b(CapacityF{current}, rs.intensity);
    println(
        "<{};{};{}> {} - {} = {}",
        left_bound,
        current,
        right_bound,
        p,
        rs.blocking_probability,
        p - rs.blocking_probability);
    if (p > rs.blocking_probability) {
      left_bound = current;
    } else if (p < rs.blocking_probability) {
      right_bound = current;
    } else {
      break;
    }
  }

  // TODO(PW): check if multiply by peakness is needed
  return CapacityF{current * get(rs.tc.size)};
}

//----------------------------------------------------------------------
Probabilities
kaufman_roberts_distribution(
    const IncomingRequestStreams &in_request_streams,
    Capacity V,
    KaufmanRobertsVariant kr_variant)
{
  Probabilities state(size_t(V) + 1);
  state[0] = Probability{1};

  rng::for_each(rng::view::closed_iota(Capacity{1}, V), [&](Capacity n) {
    for (const auto &rs : in_request_streams) {
      const auto tc_size = [&]() {
        if (kr_variant == KaufmanRobertsVariant::FixedReqSize) {
          return SizeF{rs.tc.size};
        } else {
          return SizeRescale{rs.peakedness} * rs.tc.size;
        }
      }();
      auto previous_state = Capacity{n - tc_size};
      if (previous_state >= Capacity{0}) {
        state[size_t(n)] += Intensity{get(rs.mean) / get(rs.peakedness)} * tc_size *
                            state[size_t(previous_state)];
      }
    }
    state[size_t(n)] /= n;
  });
  Math::normalize_L1(state);
  return state;
}

//----------------------------------------------------------------------
OutgoingRequestStreams
kaufman_roberts_blocking_probability(
    const IncomingRequestStreams &in_request_streams,
    CapacityF V,
    KaufmanRobertsVariant kr_variant)
{
  auto distribution =
      kaufman_roberts_distribution(in_request_streams, Capacity{V}, kr_variant);
  std::vector<OutgoingRequestStream> out_request_streams;
  for (const auto &in_rs : in_request_streams) {
    CapacityF n{V - in_rs.tc.size + Size{1}};

    auto blocking_probability =
        rng::accumulate(distribution | rng::view::drop(size_t(n)), Probability{0});

    ASSERT(
        blocking_probability >= Probability{0} && blocking_probability <= Probability{1},
        "Blocking probability should be between 0 and 1, but is equal to {}",
        blocking_probability);

    out_request_streams.emplace_back(in_rs.tc, blocking_probability, in_rs.intensity);
  }

  return out_request_streams;
}

//----------------------------------------------------------------------
OutgoingRequestStreams
compute_overflow_parameters(OutgoingRequestStreams out_request_streams, CapacityF V)
{
  for (auto &rs : out_request_streams) {
    // TODO(PW): add CLI option for choosing fittin to carried traffic
    // rs.fictional_capacity = compute_fictional_capacity_fit_carried_traffic(
    // out_request_streams, V, rs.tc.id, KaufmanRobertsVariant::FixedReqSize);
    // println(
    // "Cr V= {}, V_f = {}, P_b={}",
    // V,
    // rs.fictional_capacity,
    // extended_erlang_b(rs.fictional_capacity, rs.intensity));
    rs.fictional_capacity = compute_fictional_capacity_fit_blocking_probability2(rs, V);

    rs.variance = compute_riordan_variance(
        rs.mean, rs.intensity, rs.fictional_capacity, rs.tc.size);

    ASSERT(
        get(rs.variance) >= 0,
        "Variance should be positive but is equal to {}.",
        rs.variance);

    rs.peakedness = rs.variance / rs.mean;
  }

  return out_request_streams;
}

//----------------------------------------------------------------------
// Formula (3.35)
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
// Formula (3.34)
Probability
conditional_transition_probability(
    Capacity n, Capacity V, Count resources_number, Capacity f, Size t)
{
  auto nominator =
      combinatorial_arrangement_number(V - n, resources_number, Capacity{get(t) - 1});
  auto denominator = combinatorial_arrangement_number(V - n, resources_number, f);
  return {Probability{1} - Probability{nominator / denominator}};
}

} // namespace Model
