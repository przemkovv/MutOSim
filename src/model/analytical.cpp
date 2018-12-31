
#include "analytical.h"

#include "logger.h"
#include "model/common.h"
#include "model/group.h"
#include "overflow_far.h"
#include "scenario_settings.h"
#include "simulation/group.h"
#include "simulation/source_stream/source_stream.h"
#include "simulation/source_stream/source_stream_format.h"
#include "stream_properties_format.h"
#include "traffic_class.h"

#include <map>
#include <nlohmann/json.hpp>
#include <range/v3/action/push_back.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/action/transform.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/none_of.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <fmt/ostream.h>

namespace rng = ranges;

namespace Model
{
void
analytical_computations_hardcoded()
{
  IncomingRequestStreams traffic_classes1;
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{1}, Intensity{20.0L}, Intensity{1.0L}, Size{1}, {}});
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{2}, Intensity{10.0L}, Intensity{1.0L}, Size{2}, {}});
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{3}, Intensity{3.33333L}, Intensity{1.0L}, Size{6}, {}});
  Model::Group g1{Capacity{60}, KaufmanRobertsVariant::FixedReqSize};
  Model::Group g2{Capacity{60}, KaufmanRobertsVariant::FixedReqSize};
  Model::Group g3{Capacity{60}, KaufmanRobertsVariant::FixedReqSize};
  g1.add_incoming_request_streams(traffic_classes1);
  g2.add_incoming_request_streams(traffic_classes1);
  g3.add_incoming_request_streams(traffic_classes1);

  Model::Group g0{Capacity{42}, KaufmanRobertsVariant::FixedReqSize};
  g0.add_incoming_request_streams(g1.get_outgoing_request_streams());
  g0.add_incoming_request_streams(g2.get_outgoing_request_streams());
  g0.add_incoming_request_streams(g3.get_outgoing_request_streams());

  println("{}", g0.get_outgoing_request_streams());
}
void
analytical_computations(ScenarioSettings &scenario, KaufmanRobertsVariant kr_variant)
{
  auto &layers_types = scenario.layers_types;
  ASSERT(
      rng::none_of(
          layers_types | rng::view::values,
          [](auto layer_type) { return layer_type == LayerType::Unknown; }),
      "The current model supports only FAG and LAG with equal and unequal capacities.");

  const auto &topology = scenario.topology;

  using ModelGroups = std::map<GroupName, Model::Group>;
  using ModelGroupsPtr = std::map<GroupName, Model::Group *>;
  ModelGroups model_groups;
  std::map<Layer, ModelGroupsPtr> model_groups_layers;

  for (const auto &[group_name, group] : topology.groups) {
    ASSERT(
        layers_types.at(group->layer()) == LayerType::FullAvailability,
        "The current model doesn't support forwarding traffic to more than one next "
        "groups.");

    auto [model_group_it, inserted] =
        model_groups.emplace(group_name, Model::Group{group->capacity(), kr_variant});
    std::ignore = inserted;

    for (const auto &next_group : group->next_groups()) {
      model_group_it->second.add_next_group(next_group->name());
    }
    model_groups_layers[group->layer()].emplace(group_name, &model_group_it->second);
  }

  for (const auto &[source_name, source_stream] : topology.sources) {
    std::ignore = source_name;
    const auto &target_group = source_stream->get_target_group().name();
    model_groups.at(target_group)
        .add_incoming_request_stream(IncomingRequestStream{source_stream->tc_});
  }

  for (const auto &[layer, model_groups_ptrs] : model_groups_layers) {
    std::ignore = layer;
    for (const auto &[group_name, model_group_ptr] : model_groups_ptrs) {
      std::ignore = group_name;
      for (const auto &next_group_name : model_group_ptr->next_groups()) {
        model_groups.at(next_group_name)
            .add_incoming_request_streams(
                model_group_ptr->get_outgoing_request_streams());
      }
    }
  }

  auto &stats = scenario.stats;
  for (const auto &[layer, model_groups_ptrs] : model_groups_layers) {
    for (const auto &[group_name, model_group_ptr] : model_groups_ptrs) {
      auto &group_stats = stats[get(group_name)];
      debug_println("Layer {}, Group {}: ", layer, group_name);
      debug_println("{}", model_group_ptr->get_outgoing_request_streams());
      for (const auto &out_stream : model_group_ptr->get_outgoing_request_streams()) {
        auto &j_tc = group_stats[std::to_string(get(out_stream.tc.id))];
        j_tc["P_block"].push_back(stat_t<>{out_stream.blocking_probability});
        j_tc["peakedness"].push_back(stat_t<>{out_stream.peakedness});
        j_tc["variance"].push_back(stat_t<>{out_stream.variance});
        j_tc["mean"].push_back(stat_t<>{out_stream.mean});
        j_tc["fictitous_capacity"].push_back(get(out_stream.fictitous_capacity));
      }
    }
  }
}
void
analytical_computations(ScenarioSettings &scenario_settings)
{
  switch (scenario_settings.analytic_model) {
  case AnalyticModel::KaufmanRobertsFixedReqSize:
    analytical_computations(scenario_settings, KaufmanRobertsVariant::FixedReqSize);
    return;
  case AnalyticModel::KaufmanRobertsFixedCapacity:
    analytical_computations(scenario_settings, KaufmanRobertsVariant::FixedCapacity);
    return;
  }
}

LayerType
check_layer_type(const Simulation::Topology &topology, Layer layer)
{
  const auto &groups = topology.groups_per_layer.at(layer);
  // FullAvailability
  if (rng::all_of(
          groups, [](const auto &group) { return group->next_groups().size() <= 1; })) {
    return LayerType::FullAvailability;
  }

  auto groups_names =
      groups | rng::view::transform([](const auto &group) { return group->name(); }) |
      rng::to_vector | rng::action::sort;

  if (rng::all_of(groups, [&](const auto &group) {
        auto next_groups_names =
            group->next_groups() | rng::view::filter([layer](const auto &next_group) {
              return next_group->layer() == layer;
            }) |
            rng::view::transform(
                [](const auto &next_group) { return next_group->name(); }) |
            rng::to_vector | rng::action::push_back(group->name()) | rng::action::sort;
        return next_groups_names == groups_names;
      })) {
    if (auto capacities = groups | rng::view::transform([](const auto &group) {
                            return group->capacity();
                          }) |
                          rng::view::unique | rng::to_vector;
        capacities.size() == 1) {
      return LayerType::DistributedEqualCapacities;
    } else {
      return LayerType::DistributedUnequalCapacities;
    }
  }
  return LayerType::Unknown;
}
bool
check_model_prerequisites(const ScenarioSettings &scenario)
{
  return rng::all_of(
      scenario.topology.groups_per_layer | rng::view::keys,
      [&](const auto &layer) -> bool {
        return check_layer_type(scenario.topology, layer) != LayerType::Unknown;
      });
}

boost::container::flat_map<Layer, LayerType>
determine_layers_types(const Simulation::Topology &topology)
{
  boost::container::flat_map<Layer, LayerType> layers_types;
  for (const auto &[layer, groups] : topology.groups_per_layer) {
    std::ignore = groups;
    layers_types.emplace(layer, check_layer_type(topology, layer));
  }
  return layers_types;
}
} // namespace Model
