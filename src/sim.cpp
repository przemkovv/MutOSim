
#include "config.h"
#include "group.h"
#include "logger.h"
#include "source_stream.h"
#include "types.h"
#include "world.h"

#include <experimental/memory>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <map>
#include <optional>
#include <random>

using std::experimental::make_observer;

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
  {
    // World world{seed(), Duration(50), Duration(0.1)};
    World world{seed(), Duration(1'000'000), Duration(0.1)};

    auto group1 = std::make_unique<Group>(world, Size(3), Intensity(1.0));
    world.add_group(std::move(group1));

    world.add_source(
        std::make_unique<PoissonSourceStream>(world, Intensity(3), Size(1)));

    world.init();
    while (world.next_iteration()) {
    }

    world.print_stats();
  }
  if ((true)) {
    World world{seed(), Duration(1'000'000), Duration(0.1)};

    auto group1 = std::make_unique<Group>(world, Size(2), Intensity(1.0));
    auto group2 = std::make_unique<Group>(world, Size(1), Intensity(1.0));
    group1->add_next_group(make_observer(group2.get()));
    // world.add_group(std::make_unique<Group>(world, Size(20),
    // Intensity(1.0)));
    auto s1 =
        std::make_unique<PoissonSourceStream>(world, Intensity(3.0), Size(1));
    s1->attach_to_group(make_observer(group1.get()));

    world.add_group(std::move(group1));
    world.add_group(std::move(group2));
    world.add_source(std::move(s1));
    // world.add_source(
    // std::make_unique<PoissonSourceStream>(world, Intensity(3.18), Size(1)));

    world.init();
    while (world.next_iteration()) {
    }

    world.print_stats();
  }

  return 0;
}
