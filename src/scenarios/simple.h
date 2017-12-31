
#pragma once
#include "simulation_settings.h"
#include "types.h"
#include <vector>

SimulationSettings erlang_model(Intensity lambda, Capacity V);
SimulationSettings engset_model(Intensity gamma, Capacity V, Count N);
SimulationSettings poisson_streams(std::vector<Intensity> As,
                                   std::vector<Size> sizes,
                                   Capacity primary_V);
SimulationSettings pascal_model(Intensity gamma, Capacity V, Count N);
