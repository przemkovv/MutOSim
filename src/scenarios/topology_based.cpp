
#include "topology_based.h"

#include "simulation/group.h"
#include "simulation/overflow_policy/factory.h"
#include "simulation/source_stream/factory.h"
#include "types/types_format.h"

ScenarioSettings
prepare_scenario_global_A(const Config::Topology &config, Intensity A)
{
  ScenarioSettings sim_settings{config.name};
  Capacity V{0};

  auto &topology = sim_settings.topology;
  for (const auto &[name, group] : config.groups) {
    auto &g =
        topology.add_group(std::make_unique<Simulation::Group>(name, group.capacities[0]));

    g.set_overflow_policy(Simulation::make_overflow_policy(group.overflow_policy, g));

    for (const auto &tcs : group.traffic_classess_settings) {
      for (const auto &cr : tcs.second.compression_ratios) {
        g.add_compression_ratio(tcs.first, cr.threshold, cr.size, cr.intensity_factor);
      }
      if (tcs.second.block) {
        g.block_traffic_class(tcs.first);
      }
    }

    V += group.capacities[0];
  }
  for (const auto &[name, group] : config.groups) {
    for (const auto &connected_group : group.connected) {
      topology.connect_groups(name, connected_group);
    }
  }
  Weight sum = std::accumulate(
      begin(config.traffic_classes),
      end(config.traffic_classes),
      Weight{0},
      [](const auto x, const auto &tc) { return tc.second.weight + x; });

  for (const auto &[tc_id, tc] : config.traffic_classes) {
    std::ignore = tc_id;
    const auto ratio = tc.weight / sum;
    Intensity offered_intensity = A * V * ratio / tc.size;
    topology.add_traffic_class(tc.id, offered_intensity, tc.serve_intensity, tc.size);
  }

  for (const auto &source : config.sources) {
    auto &tc = topology.get_traffic_class(source.tc_id);
    topology.add_source(Simulation::create_stream(source.type, source, tc));
    topology.attach_source_to_group(source.name, source.attached);
  }

  return sim_settings;
}

ScenarioSettings
prepare_scenario_local_group_A(const Config::Topology &config, Intensity A)
{
  ScenarioSettings sim_settings{config.name};
  Capacity total_capacity{0};

  auto &topology = sim_settings.topology;
  for (const auto &[name, config_group] : config.groups) {
    auto &group = topology.add_group(std::make_unique<Simulation::Group>(
        name, config_group.capacities[0], config_group.layer));
    group.set_overflow_policy(
        Simulation::make_overflow_policy(config_group.overflow_policy, group));

    for (const auto &tcs : config_group.traffic_classess_settings) {
      for (const auto &cr : tcs.second.compression_ratios) {
        group.add_compression_ratio(
            tcs.first, cr.threshold, cr.size, cr.intensity_factor);
      }
      if (tcs.second.block) {
        group.block_traffic_class(tcs.first);
      }
    }

    total_capacity += config_group.capacities[0];
  }
  for (const auto &[name, config_group] : config.groups) {
    for (const auto &connected_group : config_group.connected) {
      topology.connect_groups(name, connected_group);
    }
  }

  std::unordered_map<GroupName, Weight> weights_sum_per_group;
  for (const auto &source : config.sources) {
    weights_sum_per_group[source.attached] +=
        config.traffic_classes.at(source.tc_id).weight;
  }

  Intensity traffic_intensity{0};
  for (const auto &source : config.sources) {
    const auto &cfg_tc = config.traffic_classes.at(source.tc_id);
    const auto ratio = cfg_tc.weight / weights_sum_per_group[source.attached];
    const auto &group = topology.get_group(source.attached);
    const auto intensity_multiplier = config.groups.at(group.name()).intensity_multiplier;
    Intensity offered_intensity =
        A * intensity_multiplier * group.total_capacity_ * ratio / cfg_tc.size;
    const auto &tc = topology.add_traffic_class(
        cfg_tc.id,
        offered_intensity,
        cfg_tc.serve_intensity,
        cfg_tc.size,
        cfg_tc.max_path_length);

    topology.add_source(Simulation::create_stream(source.type, source, tc));
    topology.attach_source_to_group(source.name, source.attached);

    traffic_intensity +=
        tc.source_intensity / tc.serve_intensity * tc.size / total_capacity;
  }

  sim_settings.name += fmt::format(" a={}", traffic_intensity);
  sim_settings.a = traffic_intensity;
  sim_settings.A = A;
  return sim_settings;
}
