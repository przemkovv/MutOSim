
#pragma once

#include "common.h"
#include "types/types.h"

#include <boost/container/flat_map.hpp>

struct ScenarioSettings;

namespace Simulation {
struct Topology;
}

namespace Model {
void analytical_computations_hardcoded();
void analytical_computations(ScenarioSettings &scenario_settings, KaufmanRobertsVariant kr_variant);
void analytical_computations(ScenarioSettings &scenario_settings);

LayerType check_layer_type(const Simulation::Topology &topology, Layer layer);
bool      check_model_prerequisites(const ScenarioSettings &scenario_settings);
boost::container::flat_map<Layer, LayerType>
determine_layers_types(const Simulation::Topology &topology);

} // namespace Model
