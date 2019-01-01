
#pragma once
#include "scenario_settings.h"
#include "types/types.h"

ScenarioSettings single_overflow_poisson(Intensity lambda, Capacity V);
ScenarioSettings single_overflow_engset(Intensity gamma, Capacity V, Count N);

ScenarioSettings single_overflow_poisson(Intensity A,
                                           std::vector<Capacity> primary_Vs,
                                           std::vector<std::vector<Size>> t_c_s,
                                           Capacity secondary_V);
ScenarioSettings multiple_sources_single_overflow();
