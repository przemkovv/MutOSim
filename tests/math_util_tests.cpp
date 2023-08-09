
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "math_utils.h"

// #include <catch.hpp>

TEST_CASE("factorial", "[math]")
{
  REQUIRE(1 == Math::factorial(0));
  REQUIRE(1 == Math::factorial(1));
  REQUIRE(120 == Math::factorial(5));
}

TEST_CASE("n_over_k_int64 positive n", "[math]")
{
  REQUIRE(1 == Math::n_over_k(int64_t(5), int64_t(5)));
  REQUIRE(5 == Math::n_over_k(int64_t(5), int64_t(4)));
  REQUIRE(10 == Math::n_over_k(int64_t(5), int64_t(3)));
  REQUIRE(10 == Math::n_over_k(int64_t(5), int64_t(2)));
  REQUIRE(5 == Math::n_over_k(int64_t(5), int64_t(1)));
  REQUIRE(1 == Math::n_over_k(int64_t(5), int64_t(0)));
}

TEST_CASE("n_over_k_int64 negative n", "[math]")
{
  REQUIRE(-126 == Math::n_over_k(int64_t(-5), int64_t(5)));
  REQUIRE(70 == Math::n_over_k(int64_t(-5), int64_t(4)));
  REQUIRE(-35 == Math::n_over_k(int64_t(-5), int64_t(3)));
  REQUIRE(15 == Math::n_over_k(int64_t(-5), int64_t(2)));
  REQUIRE(-5 == Math::n_over_k(int64_t(-5), int64_t(1)));
  REQUIRE(1 == Math::n_over_k(int64_t(-5), int64_t(0)));
}

TEST_CASE("n_over_k_int64 positive n - highp", "[math]")
{
  REQUIRE(1 == Math::n_over_k(highp::int_t(5), highp::int_t(5)));
  REQUIRE(5 == Math::n_over_k(highp::int_t(5), highp::int_t(4)));
  REQUIRE(10 == Math::n_over_k(highp::int_t(5), highp::int_t(3)));
  REQUIRE(10 == Math::n_over_k(highp::int_t(5), highp::int_t(2)));
  REQUIRE(5 == Math::n_over_k(highp::int_t(5), highp::int_t(1)));
  REQUIRE(1 == Math::n_over_k(highp::int_t(5), highp::int_t(0)));
}

TEST_CASE("n_over_k_int64 negative n - highp", "[math]")
{
  REQUIRE(-126 == Math::n_over_k(highp::int_t(-5), highp::int_t(5)));
  REQUIRE(70 == Math::n_over_k(highp::int_t(-5), highp::int_t(4)));
  REQUIRE(-35 == Math::n_over_k(highp::int_t(-5), highp::int_t(3)));
  REQUIRE(15 == Math::n_over_k(highp::int_t(-5), highp::int_t(2)));
  REQUIRE(-5 == Math::n_over_k(highp::int_t(-5), highp::int_t(1)));
  REQUIRE(1 == Math::n_over_k(highp::int_t(-5), highp::int_t(0)));
}

TEST_CASE("n_over_k_int64 positive n - Capacity", "[math]")
{
  using Model::Capacity;
  REQUIRE(Capacity{1} == Math::n_over_k(Capacity(5), Capacity(5)));
  REQUIRE(Capacity{5} == Math::n_over_k(Capacity(5), Capacity(4)));
  REQUIRE(Capacity{10} == Math::n_over_k(Capacity(5), Capacity(3)));
  REQUIRE(Capacity{10} == Math::n_over_k(Capacity(5), Capacity(2)));
  REQUIRE(Capacity{5} == Math::n_over_k(Capacity(5), Capacity(1)));
  REQUIRE(Capacity{1} == Math::n_over_k(Capacity(5), Capacity(0)));
}

TEST_CASE("n_over_k_int64 negative n - Capacity", "[math]")
{
  using Model::Capacity;
  REQUIRE(Capacity{-126} == Math::n_over_k(Capacity(-5), Capacity(5)));
  REQUIRE(Capacity{70} == Math::n_over_k(Capacity(-5), Capacity(4)));
  REQUIRE(Capacity{-35} == Math::n_over_k(Capacity(-5), Capacity(3)));
  REQUIRE(Capacity{15} == Math::n_over_k(Capacity(-5), Capacity(2)));
  REQUIRE(Capacity{-5} == Math::n_over_k(Capacity(-5), Capacity(1)));
  REQUIRE(Capacity{1} == Math::n_over_k(Capacity(-5), Capacity(0)));
}

TEST_CASE("lerp", "[math]")
{
  using namespace Catch::literals;
  REQUIRE(Math::lerp(0.5f, 0.0f, 1.0f) == 0.5_a);
  REQUIRE_FALSE(Math::lerp(0.5f, 0.0f, 1.0f) == 0.4_a);
  REQUIRE(Math::lerp(0.0f, 0.0f, 1.0f) == 0.0_a);
  REQUIRE(Math::lerp(1.0f, 0.0f, 1.0f) == 1.0_a);
  REQUIRE(Math::lerp(0.25f, 0.0f, 1.0f) == 0.25_a);
  REQUIRE(Math::lerp(0.75f, 0.0f, 1.0f) == 0.75_a);
  REQUIRE(Math::lerp(0.5f, 1.0f, 3.0f) == 2.0_a);
  REQUIRE_FALSE(Math::lerp(0.5f, 1.0f, 3.0f) == 1.0_a);
}

TEST_CASE("normalize_l1", "[math]")
{
  using namespace Catch::literals;
  std::vector<float> input{1, 1, 1, 1};
  const std::vector  expected_result{0.25_a, 0.25_a, 0.25_a, 0.25_a};
  const auto         expected_norm = 4.0_a;
  auto               norm = Math::normalize_L1(input);
  REQUIRE(expected_result.size() == input.size());
  REQUIRE(expected_norm == norm);
  for (size_t i = 0; i < input.size(); ++i)
  {
    REQUIRE(expected_result[i] == input[i]);
  }
}
