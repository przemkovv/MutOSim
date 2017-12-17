
#pragma once

#include "group.h"
#include "source_stream.h"
#include "topology.h"
#include "simulation_settings.h"

#include <map>
#include <memory>

uint64_t seed();

void add_groups(World &world,
                const std::vector<std::unique_ptr<Group>> &groups);

void add_sources(World &world,
                 const std::vector<std::unique_ptr<SourceStream>> &sources);

void set_topology(World &world, Topology &topology);

SimulationSettings erlang_model();
SimulationSettings engset_model();
SimulationSettings single_overflow();
