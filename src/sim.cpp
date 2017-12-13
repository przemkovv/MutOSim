
#include "config.h"
#include "group.h"
#include "logger.h"
#include "source_stream.h"
#include "types.h"
#include "world.h"

#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <map>
#include <optional>
#include <random>

uint64_t seed();

uint64_t seed()
{
  if constexpr (Config::constant_seed) {
    return 0;
  }
  std::random_device rd;
  return rd();
}

int main()
{
  World world{seed(), Duration(10'000'000), Duration(0.1)};

  world.add_group(std::make_unique<Group>(world, Size(20), Intensity(1.0)));

  world.add_source(
      std::make_unique<PoissonSourceStream>(world, Intensity(10.0), Size(1)));
  world.add_source(
      std::make_unique<PoissonSourceStream>(world, Intensity(3.18), Size(1)));

  world.init();
  while (world.next_iteration()) {
  }

  world.print_stats();

  return 0;
}
