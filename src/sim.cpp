
#include "config.h"
#include "group.h"
#include "logger.h"
#include "sim.h"
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

uint64_t seed()
{
  if constexpr (Config::constant_seed) {
    return 0;
  }
  std::random_device rd;
  return rd();
}

void add_groups(World &world, const std::vector<std::unique_ptr<Group>> &groups)
{
  for (auto &group : groups) {
    world.add_group(group.get());
  }
}

void add_sources(World &world,
                 const std::vector<std::unique_ptr<SourceStream>> &sources)
{
  for (auto &source : sources) {
    world.add_source(source.get());
  }
}

int main()
{
  const auto duration = Duration(10'000'000);
  // const auto duration = Duration(100);
  {
    World world{seed(), duration, Duration(0.1)};

    std::vector<std::unique_ptr<Group>> groups;
    std::vector<std::unique_ptr<SourceStream>> sources;
    groups.emplace_back(
        std::make_unique<Group>(world, Size(3), Intensity(1.0)));

    sources.emplace_back(
    std::make_unique<PoissonSourceStream>(world, Intensity(3.0), Size(1)));
    // sources.emplace_back(std::make_unique<EngsetSourceStream>(
        // world, Intensity(3.0), Size(4), Size(1)));
    sources[0]->attach_to_group(groups[0].get());

    add_groups(world, groups);
    add_sources(world, sources);

    world.init();
    world.run();
  }
  if ((true)) {
    World world{seed(), duration, Duration(0.1)};

    std::vector<std::unique_ptr<Group>> groups;
    std::vector<std::unique_ptr<SourceStream>> sources;

    groups.emplace_back(
        std::make_unique<Group>(world, Size(2), Intensity(1.0)));
    groups.emplace_back(
        std::make_unique<Group>(world, Size(1), Intensity(1.0)));
    sources.emplace_back(
        std::make_unique<PoissonSourceStream>(world, Intensity(3.0), Size(1)));

    groups[0]->add_next_group(groups[1].get());
    sources[0]->attach_to_group(groups[0].get());

    add_groups(world, groups);
    add_sources(world, sources);

    world.init();
    world.run();
  }

  return 0;
}
