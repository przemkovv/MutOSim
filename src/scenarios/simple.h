
#pragma once
#include "scenario_settings.h"
#include "types/types.h"

#include <vector>

ScenarioSettings
erlang_model(Simulation::Intensity lambda, Simulation::Capacity V);

ScenarioSettings engset_model(
    Simulation::Intensity lambda,
    Simulation::Capacity  V,
    Simulation::Count     N);
ScenarioSettings engset2_model(
    Simulation::Intensity lambda,
    Simulation::Capacity  V,
    Simulation::Count     N);

ScenarioSettings poisson_streams(
    std::vector<Simulation::Intensity> As,
    std::vector<Simulation::Size>      sizes,
    Simulation::Capacity               primary_V);

ScenarioSettings pascal_source_model(
    Simulation::Intensity lambda,
    Simulation::Capacity  V,
    Simulation::Count     S);

