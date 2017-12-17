
#include "calculation.h"
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

struct Topology {
  std::map<Name, std::unique_ptr<Group>> groups;
  std::map<Name, std::unique_ptr<SourceStream>> sources;

  // void connect_groups(Uuid from, Uuid to)
};

struct SimulationSettings {
  Duration duration;

  Topology topology;
};

int main()
{
  const auto duration = Duration(20'000'000);
  // const auto duration = Duration(100'000);
  // const auto duration = Duration(2000);
  if ((true)) { // Erlang model
    World world{seed(), duration, Duration(0.1)};

    const auto lambda = Intensity(3.0);
    const auto micro = Intensity(1.0);
    const auto V = Size(1);
    const auto A = lambda / micro;
    print("[Erlang] P_loss = P_block = E_V(A) = {}\n", erlang_pk(A, V, V));

    std::vector<std::unique_ptr<Group>> groups;
    std::vector<std::unique_ptr<SourceStream>> sources;
    groups.emplace_back(std::make_unique<Group>("G1", V, micro));

    sources.emplace_back(
        std::make_unique<PoissonSourceStream>("S1", lambda, Size(1)));
    sources[0]->attach_to_group(groups[0].get());

    add_groups(world, groups);
    add_sources(world, sources);

    world.init();
    world.run();
    print("[Erlang] P_loss = P_block = E_V(A) = {}\n", erlang_pk(A, V, V));
  }
  { // Engset model
    World world{seed(), duration, Duration(0.1)};

    const auto lambda = Intensity(3);
    const auto N = Size(2);
    const auto gamma = lambda / N;
    // const auto gamma = Intensity(1);
    const auto micro = Intensity(1.0);
    const auto V = Size(1);
    const auto alpha = gamma / micro;

    print("[Engset] P_block = E(alfa, V, N) = {}\n", engset_pi(alpha, V, N, V));
    print("[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
          engset_pi(alpha, V, N - 1, V));

    std::vector<std::unique_ptr<Group>> groups;
    std::vector<std::unique_ptr<SourceStream>> sources;
    groups.emplace_back(std::make_unique<Group>("G1", V, micro));

    sources.emplace_back(
        std::make_unique<EngsetSourceStream>("S1", gamma, N, Size(1)));
    sources[0]->attach_to_group(groups[0].get());

    add_groups(world, groups);
    add_sources(world, sources);

    world.init();
    world.run();
    print("[Engset] P_block = E(alfa, V, N) = {}\n", engset_pi(alpha, V, N, V));
    print("[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
          engset_pi(alpha, V, N - 1, V));
  }
  if ((false)) {
    World world{seed(), duration, Duration(0.1)};

    std::vector<std::unique_ptr<Group>> groups;
    std::vector<std::unique_ptr<SourceStream>> sources;

    groups.emplace_back(std::make_unique<Group>("G1", Size(2), Intensity(1.0)));
    groups.emplace_back(std::make_unique<Group>("G2", Size(1), Intensity(1.0)));
    sources.emplace_back(
        std::make_unique<PoissonSourceStream>("S1", Intensity(3.0), Size(1)));

    groups[0]->add_next_group(groups[1].get());
    sources[0]->attach_to_group(groups[0].get());

    add_groups(world, groups);
    add_sources(world, sources);

    world.init();
    world.run();
  }

  return 0;
}
