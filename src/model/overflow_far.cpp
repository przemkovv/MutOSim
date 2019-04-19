
#include "overflow_far.h"

#include "erlang_formula.h"
#include "logger.h"
#include "math_utils.h"
#include "model/resource_format.h"
#include "model/stream_properties_format.h"
#include "types/difference_type.h"
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

namespace Model {
//----------------------------------------------------------------------

static constexpr bool DebugTransition = false;

//----------------------------------------------------------------------
IncomingRequestStreams
convert_to_incoming_streams(
    const std::vector<OutgoingRequestStreams> &out_request_streams_per_group)
{
  // Formulas 3.17 and 3.18
  std::map<TrafficClassId, IncomingRequestStream> incoming_request_streams;
  for (const auto &out_request_streams : out_request_streams_per_group)
  {
    rng::for_each(
        out_request_streams, [&](const OutgoingRequestStream &out_rs) {
          auto [stream_it, inserted] = incoming_request_streams.emplace(
              out_rs.tc.id, IncomingRequestStream{});
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
  auto inv_sum =
      rng::accumulate(
          in_request_streams | rng::view::transform([](const auto &rs) {
            return rs.mean * Size{rs.tc.size};
          }),
          WeightF{0})
          .invert();

  // Formula 3.19
  auto peakedness = rng::accumulate(
      in_request_streams | rng::view::transform([inv_sum](const auto &rs) {
        return rs.variance * (Size{rs.tc.size} * inv_sum);
      }),
      Peakedness{0});

  return peakedness;
}

//----------------------------------------------------------------------
// Formula 3.105
Variance
compute_riordan_variance(
    const MeanIntensity mean,
    const Intensity     intensity,
    const CapacityF     fictitous_capacity,
    const SizeF         tc_size)
{
  const auto denominator = fictitous_capacity.value() / tc_size.value() + 1
                           - intensity.value() + mean.value();
  Variance::value_type variance =
      mean.value() * (intensity.value() / denominator + 1 - mean.value());
  return Variance{variance};
}

//----------------------------------------------------------------------
// Serviced traffic fit criterion (Formulas 3.8 and 3.9)
CapacityF
compute_fictitious_capacity_fit_carried_traffic(
    const OutgoingRequestStreams &out_request_streams,
    const CapacityF               V,
    const TrafficClassId          tc_id,
    const KaufmanRobertsVariant   kr_variant)
{
  return V
         - rng::accumulate(
             out_request_streams | rng::view::filter([tc_id](const auto &rs) {
               return rs.tc.id != tc_id;
             }) | rng::view::transform([&](const auto &rs) {
               if (kr_variant == KaufmanRobertsVariant::FixedCapacity)
               {
                 return rs.mean_request_number
                        * (SizeRescale{rs.peakedness} * Size{rs.tc.size});
               }
               else
               {
                 return rs.mean_request_number * Size{rs.tc.size};
               }
             }),
             CapacityF{0});
}
//----------------------------------------------------------------------

Probabilities
kaufman_roberts_distribution(
    const IncomingRequestStreams &in_request_streams,
    const Resource<CapacityF>     resource,
    const Size                    offset,
    const KaufmanRobertsVariant   kr_variant)
{
  const auto    V = Model::Capacity{resource.V()} + offset;
  Probabilities state(size_t(V) + 1, Probability{0});
  state[0] = Probability{1};

  for (Capacity n{1}; n <= V; ++n)
  {
    for (const auto &rs : in_request_streams)
    {
      const SizeF tc_size = [&]() -> SizeF {
        if (kr_variant == KaufmanRobertsVariant::FixedReqSize)
        {
          return SizeF{rs.tc.size};
        }
        else
        {
          return SizeRescale{rs.peakedness} * Size{rs.tc.size};
        }
      }();
      const auto previous_state = Capacity{CapacityF{n} - tc_size};
      if (previous_state >= Capacity{0})
      {
        Probability previous_state_value{0};

        if (auto prec = get(CapacityF{n} - tc_size);
            (false) && floor(prec) < prec)
        {
          const auto fraction = prec - floor(prec);
          const auto s1 = state[size_t(previous_state)];
          const auto s2 =
              state[std::min(size_t(previous_state) + 1, size_t(V))];
          previous_state_value =
              Probability{Math::lerp(1 - fraction, get(s1), get(s2))};
        }
        else
        {
          previous_state_value = state[size_t(previous_state)];
        }
        const auto chi = [&]() -> Probability {
          if (resource.components.size() == 1)
          {
            // NOTE(PW): distributed equal components
            const ResourceComponent<CapacityF> &component =
                resource.components.front();
            return component.number > Count{1}
                       ? conditional_transition_probability(
                           previous_state, component, Size(tc_size))
                       : Probability{1};
          }
          else
          {
            // NOTE(PW): distributed unequal components
            return conditional_transition_probability(
                previous_state, resource, Size(tc_size));
          }
        }();
        ASSERT(!isnan(get(chi)), "[{}] Chi shouldn't be nan.", location());

        state[size_t(n)] += rs.intensity * tc_size * chi * previous_state_value;
      }
    }
    state[size_t(n)] /= n;
  }
  Math::normalize_L1(state);
  return state;
}

//----------------------------------------------------------------------
OutgoingRequestStreams
kaufman_roberts_blocking_probability(
    const IncomingRequestStreams &in_request_streams,
    const Resource<CapacityF>     resource,
    const KaufmanRobertsVariant   kr_variant)
{
  CapacityF V = resource.V();
  debug_println("V: {}, Resource: {}", V, resource);
  auto distribution = kaufman_roberts_distribution(
      in_request_streams, resource, Size{0}, kr_variant);
  auto distribution2 = kaufman_roberts_distribution(
      in_request_streams, resource, Size{1}, kr_variant);
  debug_println("Distribution: {}", distribution);
  std::vector<OutgoingRequestStream> out_request_streams;
  for (const auto &in_rs : in_request_streams)
  {
    CapacityF n{std::max(CapacityF{0}, V - SizeF{in_rs.tc.size} + SizeF{1})};

    auto blocking_probability = rng::accumulate(
        distribution | rng::view::drop(size_t(floor(get(n)))), Probability{0});
    debug_println("1. n: {}, P_b: {}", n, blocking_probability);

    if (auto prec = get(n); (true) && floor(prec) < prec)
    {
      auto blocking_probability2 = rng::accumulate(
          distribution2 | rng::view::drop(size_t(ceil(get(n)))),
          Probability{0});

      WeightF interp{WeightF::value_type(prec - floor(prec))};
      // println("P1 {}\tP2 {}", blocking_probability, blocking_probability2);
      blocking_probability =
          Probability{interp.opposite() * blocking_probability
                      + interp * blocking_probability2};
    }
    if (Math::is_little_bigger(blocking_probability, Probability{1}))
    {
      debug_println(
          "3. n: {}, P_b: {}", n, blocking_probability - Probability{1});
      blocking_probability = Probability{1};
    }

    debug_println("2. n: {}, P_b: {}", n, blocking_probability);
    debug_println("{}", in_rs);

    ASSERT(
        blocking_probability >= Probability{0}
            && blocking_probability <= Probability{1},
        "[{}] Blocking probability should be between 0 and 1, but is equal "
        "to "
        "{}",
        location(),
        blocking_probability);

    auto &ors = out_request_streams.emplace_back(
        in_rs.tc, blocking_probability, in_rs.intensity);
    debug_println("{}", ors);
  }

  return out_request_streams;
}

//----------------------------------------------------------------------
OutgoingRequestStreams
compute_overflow_parameters(
    OutgoingRequestStreams out_request_streams,
    const CapacityF        V)
{
  for (auto &rs : out_request_streams)
  {
    debug_println("Computing overflow parameters for: {}", rs);
    // TODO(PW): add CLI option for choosing fittin to carried traffic
    // rs.fictitous_capacity =
    // compute_fictitious_capacity_fit_carried_traffic( out_request_streams,
    // V, rs.tc.id, KaufmanRobertsVariant::FixedReqSize); println( "Cr V= {},
    // V_f = {}, P_b={}", V, rs.fictitious_capacity,
    // extended_erlang_b(rs.fictitous_capacity, rs.intensity));
    if (rs.blocking_probability == Probability{0})
    {
      continue;
    }
    auto fictitous_capacity =
        compute_fictitious_capacity_fit_blocking_probability(rs, V);
    ASSERT(
        fictitous_capacity.has_value(),
        "[{}] Couldn't find fictitious capacity for stream {}.",
        location(),
        rs);

    rs.fictitous_capacity = fictitous_capacity.value();

    rs.variance = compute_riordan_variance(
        rs.mean, rs.intensity, rs.fictitous_capacity, SizeF{rs.tc.size});

    ASSERT(
        !isnan(get(rs.variance)),
        "[{}] Variance shouldn't be nan.",
        location());
    ASSERT(
        get(rs.variance) >= 0,
        "[{}] Variance should be positive but is equal to {}. ({})",
        location(),
        rs.variance,
        rs);

    if (rs.mean > MeanIntensity{0})
    {
      rs.peakedness = rs.variance / rs.mean;
    }
    else
    {
      rs.peakedness = Peakedness{0};
    }
    ASSERT(
        !isnan(get(rs.peakedness)),
        "[{}] Peakedness shouldn't be nan.",
        location());
    ASSERT(
        (get(rs.peakedness) >= 0),
        "[{}] Peakedness shouldn't be negative.",
        location());
    debug_println("Computed overflow parameters for: {}", rs);
  }

  return out_request_streams;
}

//----------------------------------------------------------------------
// Formula (3.35)
template <typename C>
Count
combinatorial_arrangement_number(
    const Capacity              x,
    const ResourceComponent<C> &component)
{
  Count upper_limit{
      Count::value_type{x.value() / (Capacity(component.v).value() + 1)}};
  // TODO(PW): verify what approach should be applied:
  // a) if upper limit is greater than component numbers return 0
  // b) use max(upper_limit, component.number)
  if (upper_limit > component.number)
  {
    // return Count{0}; // a)
    upper_limit = component.number; // b)
  }
  auto sum = [&] {
    Count s{0};
    for (Count iota{0}; iota <= upper_limit; ++iota)
    {
      const Count       factor1{Count::value_type{1 - 2 * (iota.value() % 2)}};
      const Count       factor2 = Math::n_over_k(component.number, iota);
      Count::value_type n = x.value() + component.number.value()
                            - iota.value() * (Capacity(component.v).value() + 1)
                            - 1;
      const Count factor3 =
          Math::n_over_k(Count{n}, component.number - Count{1});
      s += factor1 * factor2 * factor3;
    }
    return s;
  }();

  return Count{sum};
}

//----------------------------------------------------------------------

struct NCounter
{
  const size_t          N_;
  const Capacity        S_;
  std::vector<Capacity> values_;
  Capacity              sum_{0};
  bool                  finished_ = false;
  Count                 combinations_count_{0};
  Count                 combinations_number_{0};

  NCounter(Count N, Capacity S) : N_(get(N)), S_(S), values_(size_t(get(N)))
  {
    combinations_count_ =
        Math::n_over_k(N + Count{get(S_)} - Count{1}, N - Count{1});
    debug_println<DebugTransition>(
        "Combination count: {}", combinations_count_);

    values_.back() = S_;
  }

  NCounter &operator++()
  {
    for (size_t i{0}; i < N_ - 1; ++i)
    {
      if (values_[i] < S_)
      {
        ++values_[i];
        ++sum_;
        break;
      }
      else
      {
        sum_ -= values_[i];
        values_[i] = Capacity{0};
      }
    }
    values_.back() = S_ - sum_;
    debug_println<DebugTransition>(
        "Number: {}, State: {}", combinations_number_, values_);
    ++combinations_number_;
    finished_ = combinations_number_ == combinations_count_;
    return *this;
  }

  const auto &values() const { return values_; }
  bool        finished() const { return finished_; }
};

//----------------------------------------------------------------------
// Formula 3.38
template <typename C>
Count
combinatorial_arrangement_number_unequal_resources(
    const Capacity    x, // number of free allocation units
    const Resource<C> resource)
{
  debug_println<DebugTransition>("x: {}", x);
  const auto component_types_number =
      Count(resource.components.size()); // chi_s
  const auto &                       components = resource.components;
  std::vector<std::vector<Capacity>> all_coefficients;
  NCounter                           counter(component_types_number, x);
  do
  {
    all_coefficients.push_back(counter.values());
    ++counter;
  } while (!counter.finished());

  debug_println<DebugTransition>("All coefficients: {}", all_coefficients);

  Count sum{0};
  for (const auto &coefficients : all_coefficients)
  {
    Count factor{1};
    for (uint64_t i = 0; i < coefficients.size(); ++i)
    {
      factor = factor
               * combinatorial_arrangement_number(
                   Capacity{coefficients[i]}, components[i]);
      debug_println<DebugTransition>(
          "{} {} Factor {}", coefficients, i, factor);
    }
    sum += factor;
  }

  return sum;
}

template Count combinatorial_arrangement_number_unequal_resources<Capacity>(
    const Capacity           x, // number of free allocation units
    const Resource<Capacity> resource);

//----------------------------------------------------------------------
// Formula (3.34)
template <typename C>
Probability
conditional_transition_probability(
    const Capacity              n,
    const ResourceComponent<C> &component,
    const Size                  t)
{
  auto V = Capacity{component.V()};
  debug_println<DebugTransition>("N: {}, V: {}", n, V);
  auto nominator_resource =
      ResourceComponent{component.number, Capacity{t - Size{1}}};
  debug_println<DebugTransition>(
      "Components: nom: {}, denom: {}", nominator_resource, component);
  auto nominator = combinatorial_arrangement_number(V - n, nominator_resource);
  if (nominator == Count{0})
  {
    return Probability{1};
  }
  auto denominator = combinatorial_arrangement_number(V - n, component);
  debug_println<DebugTransition>("nom: {}, denom: {}", nominator, denominator);
  ASSERT(
      denominator != Count{0}, "[{}] Denominator cannot be zero.", location());
  ASSERT(
      nominator >= Count{0}, "[{}] Nominator cannot be negative.", location());
  ASSERT(
      denominator > Count{0},
      "[{}] Denominator cannot be negative.",
      location());
  return Probability{1} - Probability{nominator / denominator};
}
//----------------------------------------------------------------------
// Formula (3.37)
template <typename C>
Probability
conditional_transition_probability(
    const Capacity     n,
    const Resource<C> &resource,
    const Size         t)
{
  auto V = Capacity{resource.V()};
  debug_println<DebugTransition>("OrigV: {}, V: {}", resource.V(), V);
  // if (n == V)
  // {
  // return Probability{0};
  // }
  const auto denominator =
      combinatorial_arrangement_number_unequal_resources(V - n, resource);
  if (denominator == Count{0})
  {
    return Probability{1};
  }
  auto nominator_resource = resource;
  for (ResourceComponent<C> &component : nominator_resource.components)
  {
    component.v = C{Capacity{t - Size{1}}};
  }
  const auto nominator = combinatorial_arrangement_number_unequal_resources(
      V - n, nominator_resource);
  debug_println<DebugTransition>("nom: {}, denom: {}", nominator, denominator);
  ASSERT(
      denominator != Count{0}, "[{}] Denominator cannot be zero.", location());
  ASSERT(
      nominator >= Count{0}, "[{}] Nominator cannot be negative.", location());
  ASSERT(
      denominator > Count{0},
      "[{}] Denominator cannot be negative.",
      location());
  return Probability{1} - Probability{nominator / denominator};
}

} // namespace Model
