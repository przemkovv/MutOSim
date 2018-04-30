
#pragma once

#include <nlohmann/json.hpp>

struct ScenarioSettings;

namespace Model
{
void analytical_computations(const ScenarioSettings &scenario_settings);
nlohmann::json analytical_computations2(const ScenarioSettings &scenario_settings);

} // namespace Model
