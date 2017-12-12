
#include "config.h"
#include "events.h"
#include "request_stream.h"
#include "simulation.h"
#include "types.h"

#include <fmt/format.h>
#include <iostream>
#include <map>
#include <optional>
#include <random>


auto seed()
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
  std::mt19937_64 gen(seed());
  std::uniform_real_distribution<float> dis;

  Simulation sim{0, 10000, 3};
  PoissonRequestStream request_stream{gen, 0.25, sim.tick_length};

  int64_t events_number = 0;

  std::map<int64_t, int64_t> events_counts;

  while (sim.advance() < sim.length) {
    auto request = request_stream.get(1);

    events_number += request.count;

    ++events_counts[request.count];
  }

  fmt::print("\n{}/{}= {}\n", events_number, sim.length,
             static_cast<float>(events_number) / sim.length);

  for (auto p : events_counts) {
    std::cout << p.first << ' ' << std::string(p.second / 100, '*') << '\n';
  }

  return 0;
}
