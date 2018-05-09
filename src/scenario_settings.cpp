
#include "scenario_settings.h"

#include "simulation/group.h"
#include "simulation/source_stream/source_stream.h"
#include "simulation/world.h"
#include "utils.h"

#include <random>

uint64_t
seed(bool use_random_seed)
{
  if (!use_random_seed) {
    return 0;
  }
  std::random_device rd;
  return rd();
}

void
run_scenario(
    ScenarioSettings &scenario, const Duration duration, bool use_random_seed, bool quiet)
{
  scenario.world = std::make_unique<Simulation::World>(seed(use_random_seed), duration);
  auto &world = *scenario.world;
  world.set_topology(scenario.topology);

  world.init();
  if (scenario.do_before) {
    scenario.do_before();
  }
  world.run(quiet);
  if (scenario.do_after) {
    scenario.do_after();
  }

  scenario.stats = world.get_stats();
}

void
append_stats(nlohmann::json &target, const ScenarioSettings &scenario)
{
  target = concatenate(target, scenario.stats);
}
