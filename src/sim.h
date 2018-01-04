
#pragma once

#include "group.h"
#include "simulation_settings.h"
#include "source_stream/source_stream.h"
#include "topology.h"
#include "topology_parser.h"

#include <map>
#include <memory>

uint64_t seed();
void run_scenario(SimulationSettings &scenario, bool quiet = false);

void add_groups(World &world, const std::vector<std::unique_ptr<Group>> &groups);

void add_sources(World &world, const std::vector<std::unique_ptr<SourceStream>> &sources);

void set_topology(World &world, Topology &topology);

SimulationSettings prepare_scenario(const Config::Topology &config, Intensity A);
std::unique_ptr<SourceStream> create_stream(Config::SourceType type,
                                            const Config::Source &source,
                                            const TrafficClass &tc);
