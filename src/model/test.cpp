
#include "test.h"

// #include "analytical.h"
// #include "erlang_formula.h"
// #include "logger.h"
// #include "math_utils.h"
// #include "model/stream_properties_format.h"
// #include "overflow_far.h"
#include "types/difference_type.h"
// #include "types/types_format.h"
#include "types/types.h"

// #include <iostream>
#include <range/v3/algorithm/for_each.hpp>
// #include <range/v3/algorithm/transform.hpp>
// #include <range/v3/numeric/accumulate.hpp>
// // #include <range/v3/view/drop.hpp>
// // #include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
// #include <range/v3/view/map.hpp>
// #include <range/v3/view/transform.hpp>
#include <range/v3/utility/iterator_concepts.hpp>

namespace Model {

/*
[[maybe_unused]] static void
test1()
{
{
  Capacity x{5};
  Count    resources_number{1};
  Capacity f{4};
  println("x = {}", x);
  println("v_s = {}", resources_number);
  println("f_s = {}", f);

  println(
      "combinatorial_arrangement_number = {}",
      combinatorial_arrangement_number(x, ResourceComponent{resources_number,
f}));
}
{
  Capacity x{5};
  Count    resources_number{2};
  Capacity f{4};
  println("x = {}", x);
  println("v_s = {}", resources_number);
  println("f_s = {}", f);

  println(
      "combinatorial_arrangement_number = {}",
      combinatorial_arrangement_number(x, ResourceComponent{resources_number,
f}));
}
{
  Capacity x{5};
  Count    resources_number{1};
  Capacity f{6};
  println("x = {}", x);
  println("v_s = {}", resources_number);
  println("f_s = {}", f);

  println(
      "combinatorial_arrangement_number = {}",
      combinatorial_arrangement_number(x, ResourceComponent{resources_number,
f}));
}
}

[[maybe_unused]] static void
test2()
{
Count    resources_number{3};
Capacity f{10};
Capacity V{get(resources_number) * get(f)};
for (Capacity n{1}; n <= V; ++n)
{
  try
  {
    // Capacity n{5};
    // Capacity V{5};
    Size t{2};
    println("n = {}", n);
    println("V = {}", V);
    println("v_s = {}", resources_number);
    println("f_s = {}", f);
    println("t_c,s = {}", t);
    std::cout.flush();

    println(
        "conditional_transition_probability = {}",
        conditional_transition_probability(n,
ResourceComponent{resources_number, f}, t)); } catch (const std::exception &ex)
  {
    println(ex.what());
  }
}
}

void
test3()
{
namespace rng = ranges;
Count    resources_number{1};
Capacity f{4};
Capacity V{get(resources_number) * get(f)};
SizeF    t{2};

Probabilities state(size_t(V) + 1, Probability{0});
state[0] = Probability{1};

for (Capacity n{1}; n <= V; ++n)
{
  const auto tc_size = t;
  auto       previous_state = Capacity{n - tc_size};
  if (previous_state >= Capacity{0})
  {
    Probability previous_state_value{0};

    if (auto prec = get(n - tc_size); (false) && std::floor(prec) < prec)
    {
      const auto fraction = prec - std::floor(prec);
      const auto s1 = state[size_t(previous_state)];
      const auto s2 = state[std::min(size_t(previous_state) + 1, size_t(V))];
      previous_state_value = Probability{Math::lerp(1 - fraction, get(s1),
get(s2))};
    }
    else
    {
      previous_state_value = state[size_t(previous_state)];
    }

    try
    {
      auto chi = conditional_transition_probability(
          previous_state, ResourceComponent{resources_number, f}, Size{2});
      state[size_t(n)] += Intensity{1} * tc_size * chi * previous_state_value;
    } catch (const std::exception &ex)
    {
      println(ex.what());
    }
  }
  state[size_t(n)] /= n;
}
Math::normalize_L1(state);
println("{}", state);
}

*/
/*
void
test4()
{
  auto tc = TrafficClass{TrafficClassId{1}, Intensity{0.5}, Intensity{0.5},
Size{3}, Length{5}}; IncomingRequestStream  irs{tc}; IncomingRequestStreams
irss{irs};
  // Resource               res{Capacity{8}};
  Resource res{{
      {Count{1}, Capacity{8}},
      {Count{1}, Capacity{12}},
      {Count{1}, Capacity{4}},
  }};
  Resource res2{{
      {Count{3}, Capacity{8}},
  }};

  auto s1 = kaufman_roberts_distribution(irss, res, Size{0},
KaufmanRobertsVariant::FixedReqSize); auto s2 =
kaufman_roberts_distribution(irss, res2, Size{0},
KaufmanRobertsVariant::FixedReqSize); println("S1: {}", s1); println("S2: {}",
s2); println("S1==S2: {}", s1 == s2);
}

void
test5()
{
  analytical_computations_hardcoded();
  analytical_computations_hardcoded_components();
  analytical_computations_hardcoded_components2();
}
*/
/*
void
test6()
{
  {
    Resource<> resource{
        {Count{1}, Capacity{8}},
        {Count{1}, Capacity{8}},
        {Count{1}, Capacity{8}},
    };
    Resource<> resource2{
        {Count{3}, Capacity{8}},
    };
    Capacity x{7};
    println("x = {}", x);

    println(
        "combinatorial_arrangement_number = {}",
        combinatorial_arrangement_number(x, resource2.components[0]));
    println(
        "combinatorial_arrangement_number_unequal_resources = {}",
        combinatorial_arrangement_number_unequal_resources(x, resource));
  }
}
[[maybe_unused]] static void
test7()
{
  Resource<> resource{
      {Count{1}, Capacity{8}},
      {Count{1}, Capacity{8}},
      {Count{1}, Capacity{8}},
  };
  ResourceComponent<> component{Count{3}, Capacity{8}};
  Capacity            V{resource.V()};
  for (Capacity n{0}; n <= V; ++n)
  {
    try
    {
      // Capacity n{5};
      // Capacity V{5};
      Size t{7};
      println("n = {}", n);
      // println("V = {}", V);
      // println("t_c,s = {}", t);
      std::cout.flush();

      println(
          "conditional_transition_probability = {}",
          conditional_transition_probability(n, component, t));
      println(
          "conditional_transition_probability_not_equal = {}",
          conditional_transition_probability(n, resource, t));
    } catch (const std::exception &ex)
    {
      println(ex.what());
    }
  }
}
*/
/*
In instantiation of ‘void ranges::v3::view::closed_iota_fn::operator()(From, To)
    const[with From = TypesPrecision::Capacity<highp, use_int_ tag>;
          To = TypesPrecision::Capacity<highp, use_int_tag>;
          bool _concept_requires_510 = false;
          typename std::enable_if<(_concept_requires_445
               ||
(!(ranges::v3::concepts::models<ranges::v3::concepts::WeaklyIncrementable, T>()
                   && r anges::v3::concepts::models<
ranges::v3::concepts::WeaklyEqualityComparable, T, U>())))>::type
              * <anonymous> = 0]’:

              */
#if 1
void
test()
{
  // static_assert(std::is_same_v<
  // ranges::difference_type<Capacity>::type,
  // ts::underlying_type<Capacity>>);
  // static_assert(
  // ranges::v3::concepts::
  // models<ranges::v3::concepts::WeaklyIncrementable, Model::Capacity>());
  // static_assert(ranges::v3::concepts::models<
  // ranges::v3::concepts::WeaklyEqualityComparable,
  // Capacity,
  // Capacity>());
  // ranges::for_each(ranges::view::closed_iota(T{1}, V2), [&](T n) {
  // println("{}", n);
  // });
  // ranges::for_each(ranges::view::closed_iota(Capacity{1}, V), [&](Capacity n)
  // { println("{}", n);
  // });
  // // test1();
  // test2();
  // test3();
  // test5();
  // test7();
}
#endif

} // namespace Model
