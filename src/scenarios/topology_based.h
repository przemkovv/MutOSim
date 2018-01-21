
#pragma once

#include "simulation_settings.h"
#include "topology_parser.h"

SimulationSettings prepare_scenario_local_group_A(const Config::Topology &config,
                                                  Intensity A);
SimulationSettings prepare_scenario_global_A(const Config::Topology &config, Intensity A);
