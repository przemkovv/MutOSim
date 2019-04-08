
#pragma once
#include "scenario_settings.h"
#include "types/types.h"

ScenarioSettings
single_overflow_poisson(Simulation::Intensity lambda, Simulation::Capacity V);

ScenarioSettings single_overflow_engset(
    Simulation::Intensity gamma,
    Simulation::Capacity  V,
    Simulation::Count                 N);

ScenarioSettings single_overflow_poisson(
    Simulation::Intensity             A,
    std::vector<Simulation::Capacity> primary_Vs,
    std::vector<std::vector<Size>>    t_c_s,
    Simulation::Capacity              secondary_V);
ScenarioSettings multiple_sources_single_overflow();
