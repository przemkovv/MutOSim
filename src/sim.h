
#pragma once

#include "simulation_settings.h"
#include "topology.h"
#include "topology_parser.h"

#include <map>
#include <memory>

class SourceStream;
struct Group;
class World;

uint64_t seed(bool use_random_seed);
void run_scenario(SimulationSettings &scenario,
                  const Duration duration,
                  bool use_random_seed,
                  bool quiet = false);

void add_groups(World &world, const std::vector<std::unique_ptr<Group>> &groups);

void add_sources(World &world, const std::vector<std::unique_ptr<SourceStream>> &sources);

void set_topology(World &world, Topology &topology);


