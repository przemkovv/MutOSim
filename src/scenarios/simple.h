
#pragma once
#include "scenario_settings.h"
#include "types.h"

#include <vector>

ScenarioSettings erlang_model(Intensity lambda, Capacity V);
ScenarioSettings engset_model(Intensity lambda, Capacity V, Count N);
ScenarioSettings engset2_model(Intensity lambda, Capacity V, Count N);
ScenarioSettings
poisson_streams(std::vector<Intensity> As, std::vector<Size> sizes, Capacity primary_V);
ScenarioSettings pascal_source_model(Intensity lambda, Capacity V, Count S);
