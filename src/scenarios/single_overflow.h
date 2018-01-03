
#pragma once
#include "simulation_settings.h"
#include "types.h"

SimulationSettings single_overflow_poisson(Intensity lambda, Capacity V);
SimulationSettings single_overflow_engset(Intensity gamma, Capacity V, Count N);

SimulationSettings single_overflow_poisson(Intensity A,
                                           std::vector<Capacity> primary_Vs,
                                           std::vector<std::vector<Size>> t_c_s,
                                           Capacity secondary_V);
SimulationSettings multiple_sources_single_overflow();
