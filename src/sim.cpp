
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
#include <memory>
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
  const auto duration = Duration(1'000'000);
  {
    World world{seed(), duration, Duration(0.1)};

    std::vector<std::unique_ptr<Group>> groups;
    groups.emplace_back(
        std::make_unique<Group>(world, Size(3), Intensity(1.0)));

    std::vector<std::unique_ptr<SourceStream>> sources;
    sources.emplace_back(
        std::make_unique<PoissonSourceStream>(world, Intensity(3.0), Size(1)));

    sources[0]->attach_to_group(groups[0].get());

    for (auto &group : groups) {
      world.add_group(group.get());
    }
    for (auto &source : sources) {
      world.add_source(source.get());
    }
    world.init();

    double stats_freq = 0.2;
    int i = 1;
    while (world.next_iteration()) {
      if (world.get_progress() > stats_freq * i) {
        world.print_stats();
        ++i;
      }
    }

    world.print_stats();
  }
  if ((true)) {
    World world{seed(), duration, Duration(0.1)};

    std::vector<Group> groups {
      {world, Size(2), Intensity(1.0)},
      {world, Size(1), Intensity(1.0)}
    };
    auto group1 = std::make_unique<Group>(world, Size(2), Intensity(1.0));
    auto group2 = std::make_unique<Group>(world, Size(1), Intensity(1.0));
    group1->add_next_group(group2.get());
    auto s1 =
        std::make_unique<PoissonSourceStream>(world, Intensity(3.0), Size(1));
    s1->attach_to_group(group1.get());

    world.add_group(group1.get());
    world.add_group(group2.get());
    world.add_source(s1.get());

    world.init();
    double stats_freq = 0.2;
    int i = 1;
    while (world.next_iteration()) {
      if (world.get_progress() > stats_freq * i) {
        world.print_stats();
        ++i;
      }
    }

    world.print_stats();
  }

  return 0;
}
