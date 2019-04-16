#include "model/overflow_far.h"

#include <catch.hpp>

using namespace Model;

TEST_CASE("combinatorial_arrangement_number, v_s=1, f_s=5", "[overflow_far]")
{
  ResourceComponent<Capacity> component{Count{1}, Capacity{5}};

  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{5}, component));
  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{4}, component));
  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{3}, component));
  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{2}, component));
  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{1}, component));
  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{0}, component));
}

TEST_CASE("combinatorial_arrangement_number, v_s=2, f_s=3", "[overflow_far]")
{
  ResourceComponent<Capacity> component{Count{2}, Capacity{3}};

  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{0}, component));
  REQUIRE(Count{2} == combinatorial_arrangement_number(Capacity{1}, component));
  REQUIRE(Count{3} == combinatorial_arrangement_number(Capacity{2}, component));
  REQUIRE(Count{4} == combinatorial_arrangement_number(Capacity{3}, component));
  REQUIRE(Count{3} == combinatorial_arrangement_number(Capacity{4}, component));
  REQUIRE(Count{2} == combinatorial_arrangement_number(Capacity{5}, component));
  REQUIRE(Count{1} == combinatorial_arrangement_number(Capacity{6}, component));
}

TEST_CASE(
    "combinatorial_arrangement_number_unequal, v_1=1, f_1=5",
    "[overflow_far]")
{
  Resource<Capacity> resource{Capacity{5}};

  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{0}, resource));
  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{1}, resource));
  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{2}, resource));
  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{3}, resource));
  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{4}, resource));
  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{5}, resource));
}

TEST_CASE(
    "combinatorial_arrangement_number_unequal, v_1=1, f_1=3, v_2=1, f_2=3",
    "[overflow_far]")
{
  Resource<Capacity> resource{Capacity{3}, Capacity{3}};

  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{0}, resource));
  REQUIRE(
      Count{2}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{1}, resource));
  REQUIRE(
      Count{3}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{2}, resource));
  REQUIRE(
      Count{4}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{3}, resource));
  REQUIRE(
      Count{3}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{4}, resource));
  REQUIRE(
      Count{2}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{5}, resource));
  REQUIRE(
      Count{1}
      == combinatorial_arrangement_number_unequal_resources(
          Capacity{6}, resource));
}
