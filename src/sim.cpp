
#include "config.h"
#include "events.h"
#include "source_stream.h"
#include "world.h"
#include "types.h"

#include <fmt/format.h>
#include <iostream>
#include <map>
#include <optional>
#include <random>

// template <>
// PoissonSourceStream<std::mt19937_64>;

uint64_t seed();

uint64_t seed()
{
  if constexpr (Config::constant_seed) {
    return 0;
  } else {
    std::random_device rd;
    return rd();
  }
}

int main()
{

  World world{seed(), 0, 10000, 3};
  PoissonSourceStream request_stream(world, 0.25, world.tick_length);

  int64_t events_number = 0;

  std::map<int64_t, uint64_t> events_counts;

  while (world.advance() < world.length) {
    auto request = request_stream.get(1);

    events_number += request.count;

    ++events_counts[request.count];
  }

  fmt::print("\n{}/{}= {}\n", events_number, world.length,
             static_cast<float>(events_number) / world.length);

  for (auto p : events_counts) {
    std::cout << p.first << ' ' << std::string(p.second / 100, '*') << '\n';
  }

  return 0;
}
