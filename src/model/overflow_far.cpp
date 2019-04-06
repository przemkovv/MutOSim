

#include "overflow_far.h"

#include "erlang_formula.h"
#include "logger.h"
#include "math_utils.h"
#include "model/stream_properties_format.h"
#include "types/types_format.h"

#include <iostream>
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
namespace ranges {
inline namespace v3 {
template <>
struct difference_type<::Count>
{
  using type = ::ts::underlying_type<::Count>;
};
template <>
struct difference_type<::Capacity>
{
  using type = ::ts::underlying_type<::Capacity>;
};
} // namespace v3
} // namespace ranges

namespace Model {
//----------------------------------------------------------------------
//
Capacity
ResourceComponent::V() const
{
  return number * v;
}

Capacity
Resource::V() const
{
  return rng::accumulate(
      components, Capacity{0}, [](const Capacity &V, const ResourceComponent &rc) {
        return V + rc.V();
      });
}

//----------------------------------------------------------------------
IncomingRequestStreams
convert_to_incoming_streams(
    const std::vector<OutgoingRequestStreams> &out_request_streams_per_group)
{
  // Formulas 3.17 and 3.18
  std::map<TrafficClassId, IncomingRequestStream> incoming_request_streams;
  for (const auto &out_request_streams : out_request_streams_per_group)
  {
    rng::for_each(out_request_streams, [&](const OutgoingRequestStream &out_rs) {
      auto [stream_it, inserted] =
          incoming_request_streams.emplace(out_rs.tc.id, IncomingRequestStream{});
      std::ignore = inserted;
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
    MeanIntensity mean,
    Intensity     intensity,
    CapacityF     fictitous_capacity,
    SizeF         tc_size)
{
  /* clang-format off */
  return Variance{
    get(mean) * (get(intensity) /
                 (
                  (
                   get(fictitous_capacity) / get(tc_size)
                  ) + 1 - get(intensity) + get(mean)
                 ) + 1 - get(mean)
                )
  };
  /* clang-format on */
}

//----------------------------------------------------------------------
// Serviced traffic fit criterion (Formulas 3.8 and 3.9)
CapacityF
compute_fictitious_capacity_fit_carried_traffic(
    const OutgoingRequestStreams &out_request_streams,
    CapacityF                     V,
    TrafficClassId                tc_id,
    KaufmanRobertsVariant         kr_variant)
{
  return V
         - rng::accumulate(
             out_request_streams | rng::view::filter([tc_id](const auto &rs) {
               return rs.tc.id != tc_id;
             }) | rng::view::transform([&](const auto &rs) {
               if (kr_variant == KaufmanRobertsVariant::FixedCapacity)
               {
                 return rs.mean_request_number * (SizeRescale{rs.peakedness} * rs.tc.size);
               }
               else
               {
                 return rs.mean_request_number * rs.tc.size;
               }
             }),
             CapacityF{0});
}
//----------------------------------------------------------------------

Probabilities
kaufman_roberts_distribution(
    const IncomingRequestStreams &in_request_streams,
    Resource                      resource,
    KaufmanRobertsVariant         kr_variant)
{
  const auto    V = resource.V();
  Probabilities state(size_t(V) + 1, Probability{0});
  state[0] = Probability{1};

  rng::for_each(rng::view::closed_iota(Capacity{1}, V), [&](Capacity n) {
    for (const auto &rs : in_request_streams)
    {
      const auto tc_size = [&]() {
        if (kr_variant == KaufmanRobertsVariant::FixedReqSize)
        {
          return SizeF{rs.tc.size};
        }
        else
        {
          return SizeRescale{rs.peakedness} * rs.tc.size;
        }
      }();
      auto previous_state = Capacity{n - tc_size};
      if (previous_state >= Capacity{0})
      {
        Probability previous_state_value{0};

        if (auto prec = get(n - tc_size); (false) && std::floor(prec) < prec)
        {
          const auto fraction = prec - std::floor(prec);
          const auto s1 = state[size_t(previous_state)];
          const auto s2 = state[std::min(size_t(previous_state) + 1, size_t(V))];
          previous_state_value = Probability{Math::lerp(1 - fraction, get(s1), get(s2))};
        }
        else
        {
          previous_state_value = state[size_t(previous_state)];
        }
        ASSERT(resource.components.size() == 1, "Support only for multiple equal components.");
        auto &component = resource.components[0];
        auto  chi =
            component.number > Count{1}
                ? conditional_transition_probability(previous_state, component, Size(tc_size))
                : Probability{1};

        state[size_t(n)] += rs.intensity * tc_size * chi * previous_state_value;
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
    CapacityF                     V,
    KaufmanRobertsVariant         kr_variant)
{
  auto distribution =
      kaufman_roberts_distribution(in_request_streams, Resource{Capacity{V}}, kr_variant);
  auto distribution2 =
      kaufman_roberts_distribution(in_request_streams, Resource{Capacity{V} + Size{1}}, kr_variant);
  std::vector<OutgoingRequestStream> out_request_streams;
  for (const auto &in_rs : in_request_streams)
  {
    CapacityF n{V - in_rs.tc.size + Size{1}};

    auto blocking_probability =
        rng::accumulate(distribution | rng::view::drop(size_t(std::floor(get(n)))), Probability{0});

    if (auto prec = get(n); (true) && std::floor(prec) < prec)
    {
      auto blocking_probability2 = rng::accumulate(
          distribution2 | rng::view::drop(size_t(std::ceil(get(n)))), Probability{0});

      auto interp = prec - std::floor(prec);
      // println("P1 {}\tP2 {}", blocking_probability, blocking_probability2);
      blocking_probability = Probability{(1 - interp) * get(blocking_probability)
                                         + interp * get(blocking_probability2)};
      // println("n {}, interp {}\tP3 {}", n, interp, blocking_probability);
    }

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
  for (auto &rs : out_request_streams)
  {
    // TODO(PW): add CLI option for choosing fittin to carried traffic
    // rs.fictitous_capacity = compute_fictitious_capacity_fit_carried_traffic(
    // out_request_streams, V, rs.tc.id, KaufmanRobertsVariant::FixedReqSize);
    // println(
    // "Cr V= {}, V_f = {}, P_b={}",
    // V,
    // rs.fictitious_capacity,
    // extended_erlang_b(rs.fictitous_capacity, rs.intensity));
    auto fictitous_capacity = compute_fictitious_capacity_fit_blocking_probability(rs, V);
    ASSERT(fictitous_capacity.has_value(), "Couldn't find fictitious capacity for stream {}.", rs);

    rs.fictitous_capacity = fictitous_capacity.value();

    rs.variance =
        compute_riordan_variance(rs.mean, rs.intensity, rs.fictitous_capacity, rs.tc.size);

    ASSERT(
        get(rs.variance) >= 0,
        "Variance should be positive but is equal to {}. ({})",
        rs.variance,
        rs);

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
  if (upper_limit > resources_number)
  {
    return Count{0};
  }
  auto sum = rng::accumulate(
      rng::view::closed_iota(Count{0}, upper_limit) | rng::view::transform([&](Count iota) {
        auto factor1 = (1 - 2 * (get(iota) % 2));
        // println("iota {}, f1 {}", iota, factor1);
        std::cout.flush();
        auto factor2 = Math::n_over_k(resources_number, iota);
        auto factor3 = Math::n_over_k(
            Count{get(x) + get(resources_number) - get(iota) * (get(f) + 1) - 1},
            resources_number - Count{1});
        return Count{factor1 * get(factor2) * get(factor3)};
      }),
      Count{0});

  return Count{sum};
}
//----------------------------------------------------------------------
// Formula 3.38
Count
combinatorial_arrangement_number_unequal_resources(
    Capacity /*x*/, // number of free allocation units
    std::vector<Count> components_numbers,
    std::vector<Capacity> /*components_capacities*/)
{
  const auto component_types_number = components_numbers.size(); // chi_s

  std::ignore = component_types_number;

  return Count{0};
}

//----------------------------------------------------------------------
// Formula (3.34)
Probability
conditional_transition_probability(
    Capacity n,
    Capacity V,
    Count    resources_number,
    Capacity f,
    Size     t)
{
  auto nominator = combinatorial_arrangement_number(V - n, resources_number, Capacity{get(t) - 1});
  auto denominator = combinatorial_arrangement_number(V - n, resources_number, f);
  return Probability{1} - Probability{nominator / denominator};
}

Probability
conditional_transition_probability(Capacity n, const ResourceComponent &component, Size t)
{
  auto V = component.V();
  auto nominator = combinatorial_arrangement_number(V - n, component.number, Capacity{get(t) - 1});
  auto denominator = combinatorial_arrangement_number(V - n, component.number, component.v);
  return Probability{1} - Probability{nominator / denominator};
}

} // namespace Model
