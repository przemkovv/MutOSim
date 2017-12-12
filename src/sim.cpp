
#include "config.h"
#include "events.h"
#include "source_stream.h"
#include "types.h"
#include "world.h"
#include "group.h"

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
  World world{seed(), Time(0), TimePeriod(100), TimePeriod(1)};
  PoissonSourceStream request_stream(world, 0.25, Size(2), world.tick_length);

  int64_t events_number = 0;

  std::map<uint64_t, uint64_t> events_counts;

  auto group = std::make_unique<Group>(world);
  world.add_group(std::move(group));

  while (world.advance() < world.length) {
    auto loads = request_stream.get(world.time);

    events_number += loads.size();

    ++events_counts[loads.size()];

    for (auto &load : loads) {
      world.serve_load(load);
    }
  }

  fmt::print("\n{}/{}= {}\n", events_number, world.length,
             static_cast<float>(events_number) / world.length);

  for (auto p : events_counts) {
    std::cout << p.first << ' ' << std::string(p.second / 1, '*') << '\n';
  }

  return 0;
}
