
#pragma once

#include "scenario_settings.h"
#include "topology_parser.h"

ScenarioSettings prepare_scenario_local_group_A(
    const Config::Topology &config,
    Simulation::Intensity   A);
ScenarioSettings prepare_scenario_global_A(
    const Config::Topology &config,
    Simulation::Intensity   A);
