
#include "config.h"
#include "events.h"
#include "group.h"
#include "source_stream.h"
#include "types.h"
#include "world.h"

#include <fmt/format.h>
#include <iostream>
#include <map>
#include <optional>
#include <random>

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
  World world{seed(), Duration(1000000), Duration(1)};

  auto group = std::make_unique<Group>(world);
  world.add_group(std::move(group));

  world.add_source(
      std::make_unique<PoissonSourceStream>(world, Intensity(1), Size(1)));

  world.init();
  while (world.next_iteration()) {
  }

  world.print_stats();

  return 0;
}
