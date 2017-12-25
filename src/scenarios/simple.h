
#pragma once
#include "types.h"
#include "simulation_settings.h"

SimulationSettings erlang_model(Intensity lambda, Capacity V);
SimulationSettings engset_model(Intensity gamma, Capacity V, Count N);
