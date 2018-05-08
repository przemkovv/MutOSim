
#pragma once

#include <nlohmann/json.hpp>

struct ScenarioSettings;

namespace Model
{
void analytical_computations_hardcoded();
void analytical_computations(ScenarioSettings &scenario_settings);

} // namespace Model
