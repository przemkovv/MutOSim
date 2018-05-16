
#include "kaufman_roberts.h"

#include "logger.h"
#include "math_utils.h"
#include "stream_properties.h"

#include <boost/math/special_functions/gamma.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <fmt/ostream.h>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>

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

//----------------------------------------------------------------------
namespace Model
{
Probabilities
kaufman_roberts_distribution(Intensity A, Size tc_size, Capacity V)
{
  using namespace boost::multiprecision;
  using namespace boost::math;
  std::vector<number<cpp_dec_float<30>>> state(size_t(V) + 1);
  number<cpp_dec_float<30>> size = get(tc_size);
  number<cpp_dec_float<30>> a = get(A);

  state[0] = 1;

  rng::for_each(rng::view::closed_iota(Capacity{1}, V), [&](Capacity n) {
    auto previous_state = Capacity{n - tc_size};
    if (previous_state >= Capacity{0}) {
      state[size_t(n)] += a * size * state[size_t(previous_state)];
    }
    state[size_t(n)] /= get(n);
  });
  Math::normalize_L1(state);


  Probabilities state2(size_t(V) + 1);
  for (size_t i = 0; i < state.size(); ++i) {
    state2[i] = Probability{probability_t{state[i]}};
  }
  return state2;
}

//----------------------------------------------------------------------
Probability
kaufman_roberts_blocking_probability(Intensity A, Size tc_size, CapacityF V)
{
  // A = Intensity(get(A)*100);
  // V = CapacityF(get(V)*100);
  // tc_size = Size(get(tc_size)*100);
  auto distribution = kaufman_roberts_distribution(A, tc_size, Capacity{V});
  CapacityF n{V - tc_size + Size{1}};

  auto blocking_probability =
      rng::accumulate(distribution | rng::view::drop(size_t(n)), Probability{0});

  ASSERT(
      blocking_probability >= Probability{0} && blocking_probability <= Probability{1},
      "Blocking probability should be between 0 and 1, but is equal to {}",
      blocking_probability);

  return blocking_probability;
}

} // namespace Model
