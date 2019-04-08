
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

namespace rng = ranges;

namespace Model {

//----------------------------------------------------------------------
void
analytical_computations_hardcoded()
{
  IncomingRequestStreams traffic_classes1;
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{1},
                                             Simulation::Intensity{20.0L},
                                             Simulation::Intensity{1.0L},
                                             Size{1},
                                             {}});
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{2},
                                             Simulation::Intensity{10.0L},
                                             Simulation::Intensity{1.0L},
                                             Size{2},
                                             {}});
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{3},
                                             Simulation::Intensity{3.33333L},
                                             Simulation::Intensity{1.0L},
                                             Size{6},
                                             {}});
  Model::Group g1{{Capacity{60}}, KaufmanRobertsVariant::FixedCapacity};
  Model::Group g2{{Capacity{60}}, KaufmanRobertsVariant::FixedCapacity};
  Model::Group g3{{Capacity{60}}, KaufmanRobertsVariant::FixedCapacity};
  g1.add_incoming_request_streams(traffic_classes1);
  g2.add_incoming_request_streams(traffic_classes1);
  g3.add_incoming_request_streams(traffic_classes1);

  Model::Group g0{Capacity{42}, KaufmanRobertsVariant::FixedCapacity};
  g0.add_incoming_request_streams(g1.get_outgoing_request_streams());
  g0.add_incoming_request_streams(g2.get_outgoing_request_streams());
  g0.add_incoming_request_streams(g3.get_outgoing_request_streams());

  println("{}", g1.get_outgoing_request_streams());
  println("{}", g0.get_outgoing_request_streams());
}

//----------------------------------------------------------------------
void
analytical_computations_hardcoded_components()
{
  IncomingRequestStreams traffic_classes1;
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{1},
                                             Simulation::Intensity{20.0L},
                                             Simulation::Intensity{1.0L},
                                             Size{1},
                                             {}});
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{2},
                                             Simulation::Intensity{10.0L},
                                             Simulation::Intensity{1.0L},
                                             Size{2},
                                             {}});
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{3},
                                             Simulation::Intensity{3.33333L},
                                             Simulation::Intensity{1.0L},
                                             Size{6},
                                             {}});
  Model::Group g1{{Count{3}, Capacity{60}},
                  KaufmanRobertsVariant::FixedCapacity};
  g1.add_incoming_request_streams(traffic_classes1);
  g1.add_incoming_request_streams(traffic_classes1);
  g1.add_incoming_request_streams(traffic_classes1);

  Model::Group g0{Capacity{42}, KaufmanRobertsVariant::FixedCapacity};
  g0.add_incoming_request_streams(g1.get_outgoing_request_streams());

  println("{}", g1.get_outgoing_request_streams());
  println("{}", g0.get_outgoing_request_streams());
}

//----------------------------------------------------------------------
void
analytical_computations_hardcoded_components2()
{
  IncomingRequestStreams traffic_classes1;
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{1},
                                             Simulation::Intensity{20.0L},
                                             Simulation::Intensity{1.0L},
                                             Size{1},
                                             {}});
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{2},
                                             Simulation::Intensity{10.0L},
                                             Simulation::Intensity{1.0L},
                                             Size{2},
                                             {}});
  traffic_classes1.emplace_back(TrafficClass{TrafficClassId{3},
                                             Simulation::Intensity{3.33333L},
                                             Simulation::Intensity{1.0L},
                                             Size{6},
                                             {}});
  Model::Group g1{{
                      {Count{1}, Capacity{60}},
                      {Count{1}, Capacity{60}},
                      {Count{1}, Capacity{60}},
                  },
                  KaufmanRobertsVariant::FixedCapacity};
  g1.add_incoming_request_streams(traffic_classes1);
  g1.add_incoming_request_streams(traffic_classes1);
  g1.add_incoming_request_streams(traffic_classes1);

  Model::Group g0{Capacity{42}, KaufmanRobertsVariant::FixedCapacity};
  g0.add_incoming_request_streams(g1.get_outgoing_request_streams());

  println("{}", g1.get_outgoing_request_streams());
  println("{}", g0.get_outgoing_request_streams());
}

std::vector<Model::Capacity>
to_model(const std::vector<Simulation::Capacity> &capacities)
{
  return capacities | rng::view::transform([](const auto &c) {
           return Model::Capacity{c};
         })
         | rng::to_vector;
}
Model::Capacity
to_model(const Simulation::Capacity &capacity)
{
  return Model::Capacity{capacity};
}
//----------------------------------------------------------------------
void
analytical_computations(
    ScenarioSettings &    scenario,
    KaufmanRobertsVariant kr_variant)
{
  auto &layers_types = scenario.layers_types;
  ASSERT(
      rng::none_of(
          layers_types | rng::view::values,
          [](auto layer_type) { return layer_type == LayerType::Unknown; }),
      "The current model supports only FAG and LAG with equal and unequal "
      "capacities.");

  const auto &topology = scenario.topology;

  using ModelGroups = std::map<GroupName, Model::Group>;
  using ModelGroupsPtr = std::map<GroupName, Model::Group *>;
  using SimGroupsPtr = std::map<GroupName, Simulation::Group *>;
  ModelGroups                     model_groups;
  std::map<Layer, ModelGroupsPtr> model_groups_layers;
  std::map<Layer, SimGroupsPtr>   simulation_groups_layers;
  std::map<GroupName, GroupName>  simulation_to_model_group;

  for (const auto &[group_name, group] : topology.groups)
  {
    auto layer = group->layer();
    simulation_groups_layers[layer].emplace(group_name, group.get());
  }

  for (auto &[layer, simulation_groups] : simulation_groups_layers)
  {
    debug_println(
        "Processing {} layer, type: {}",
        layer,
        static_cast<int>(layers_types.at(layer)));
    switch (layers_types.at(layer))
    {
      case LayerType::FullAvailability:
      {
        for (const auto &[group_name, group] : simulation_groups)
        {
          ASSERT(
              layers_types.at(layer) == LayerType::FullAvailability,
              "The current model doesn't support forwarding traffic to more "
              "than one next "
              "groups.");

          auto [model_group_it, inserted] = model_groups.emplace(
              group_name,
              Model::Group{to_model(group->capacity()), kr_variant});
          simulation_to_model_group.emplace(group_name, group_name);
          std::ignore = inserted;

          for (const auto &next_group : group->next_groups())
          {
            model_group_it->second.add_next_group(next_group->name());
          }
          model_groups_layers[layer].emplace(
              group_name, &model_group_it->second);
        }
        break;
      }
      case LayerType::DistributedUnequalCapacities:
      {
        Resource<> resource;
        auto       layer_name = fmt::format("L{}:", layer);
        for (const auto &[group_name, group] : simulation_groups)
        {
          ASSERT(
              group->capacity().size() == 1,
              "[{}] Simulation groups with multiple subgroups are not "
              "supported.",
              location());

          resource.add_component(to_model(group->total_capacity()));
          layer_name += fmt::format("{};", group_name);
        }
        GroupName current_layer_name{layer_name};
        auto [model_group_it, inserted] = model_groups.emplace(
            current_layer_name, Model::Group{resource, kr_variant});
        for (const auto &[group_name, group] : simulation_groups)
        {
          simulation_to_model_group.emplace(group_name, current_layer_name);
        }
        for (const auto &[group_name, group] : simulation_groups)
        {
          for (const auto &next_group : group->next_groups())
          {
            if (auto it = simulation_to_model_group.find(next_group->name());
                it != end(simulation_to_model_group))
            {
              if (it->second != current_layer_name)
              {
                model_group_it->second.add_next_group(next_group->name());
              }
            }
            else
            {
              model_group_it->second.add_next_group(next_group->name());
            }
          }
        }
        model_groups_layers[layer].emplace(
            current_layer_name, &model_group_it->second);
        break;
      }
      case LayerType::DistributedEqualCapacities:
      {
        ASSERT(false, "[{}] Not implemented yet.", location());
        break;
      }
      case LayerType::Unknown:
      {
        ASSERT(
            layers_types.at(layer) == LayerType::Unknown,
            "[{}] The type of layer is unknown. Cannot handle this case.",
            location());
        break;
      }
    }
  }
  debug_println("Sim to model groups mapping: {}", simulation_to_model_group);

  for (const auto &[source_name, source_stream] : topology.sources)
  {
    std::ignore = source_name;
    const auto &target_group = source_stream->get_target_group().name();
    model_groups.at(simulation_to_model_group.at(target_group))
        .add_incoming_request_stream(IncomingRequestStream{source_stream->tc_});
  }

  for (const auto &[layer, model_groups_ptrs] : model_groups_layers)
  {
    std::ignore = layer;
    for (const auto &[group_name, model_group_ptr] : model_groups_ptrs)
    {
      debug_println(
          fg(fmt::color::green),
          "Processing streams of '{}' group.",
          group_name);
      for (const auto &next_group_name : model_group_ptr->next_groups())
      {
        debug_println(
            fg(fmt::color::green),
            "Forwarding streams to '{}' group.",
            next_group_name);
        model_groups.at(simulation_to_model_group.at(next_group_name))
            .add_incoming_request_streams(
                model_group_ptr->get_outgoing_request_streams());
      }
    }
  }

  auto &stats = scenario.stats;
  for (const auto &[layer, model_groups_ptrs] : model_groups_layers)
  {
    for (const auto &[group_name, model_group_ptr] : model_groups_ptrs)
    {
      auto &group_stats = stats[get(group_name)];
      debug_println("Layer {}, Group '{}': ", layer, group_name);
      debug_println("{}", model_group_ptr->get_outgoing_request_streams());
      for (const auto &out_stream :
           model_group_ptr->get_outgoing_request_streams())
      {
        auto &j_tc = group_stats[std::to_string(get(out_stream.tc.id))];
        j_tc["P_block"].push_back(
            stat_t<>{get(out_stream.blocking_probability)});
        j_tc["peakedness"].push_back(stat_t<>{get(out_stream.peakedness)});
        j_tc["variance"].push_back(stat_t<>{get(out_stream.variance)});
        j_tc["mean"].push_back(stat_t<>{get(out_stream.mean)});
        j_tc["fictitous_capacity"].push_back(
            stat_t<>(get(out_stream.fictitous_capacity)));
      }
    }
  }
} // namespace Model
//----------------------------------------------------------------------
void
analytical_computations(ScenarioSettings &scenario_settings)
{
  switch (scenario_settings.analytic_model)
  {
    case AnalyticModel::KaufmanRobertsFixedReqSize:
      analytical_computations(
          scenario_settings, KaufmanRobertsVariant::FixedReqSize);
      return;
    case AnalyticModel::KaufmanRobertsFixedCapacity:
      analytical_computations(
          scenario_settings, KaufmanRobertsVariant::FixedCapacity);
      return;
  }
}

//----------------------------------------------------------------------
// Based on the types of groups in the layer, tries to determine if either a
// whole layer can be considered as:
// - Full Availability Group,
// - Limited Availability Group with components of equal size ,
// - Limited Availability Group with components of various sizes.
LayerType
check_layer_type(const Simulation::Topology &topology, Layer layer)
{
  const auto &groups = topology.groups_per_layer.at(layer);
  // FullAvailability
  if (rng::all_of(groups, [](const auto &group) {
        return group->next_groups().size() <= 1;
      }))
  {
    debug_println("[Analytical] Layer: {}, FullAvailability", layer);
    return LayerType::FullAvailability;
  }

  auto groups_names =
      groups
      | rng::view::transform([](const auto &group) { return group->name(); })
      | rng::to_vector | rng::action::sort;

  if (rng::all_of(groups, [&](const auto &group) {
        auto next_groups_names =
            group->next_groups()
            | rng::view::filter([layer](const auto &next_group) {
                return next_group->layer() == layer;
              })
            | rng::view::transform(
                [](const auto &next_group) { return next_group->name(); })
            | rng::to_vector | rng::action::push_back(group->name())
            | rng::action::sort;
        return next_groups_names == groups_names;
      }))
  {
    if (auto capacities = groups | rng::view::transform([](const auto &group) {
                            return group->capacity();
                          })
                          | rng::view::unique | rng::to_vector;
        capacities.size() == 1)
    {
      debug_println(
          "[Analytical] Layer: {}, DistributedEqualCapacities", layer);
      return LayerType::DistributedEqualCapacities;
    }
    else
    {
      debug_println(
          "[Analytical] Layer: {}, DistributedUnequalCapacities", layer);
      return LayerType::DistributedUnequalCapacities;
    }
  }
  debug_println("[Analytical] Layer: {}, Unkown", layer);
  return LayerType::Unknown;
}

//----------------------------------------------------------------------
// Check if we can assign a proper analytical model to each of the layers.
bool
check_model_prerequisites(const ScenarioSettings &scenario)
{
  return rng::all_of(
      scenario.topology.groups_per_layer | rng::view::keys,
      [&](const auto &layer) -> bool {
        return check_layer_type(scenario.topology, layer) != LayerType::Unknown;
      });
}
//----------------------------------------------------------------------

boost::container::flat_map<Layer, LayerType>
determine_layers_types(const Simulation::Topology &topology)
{
  boost::container::flat_map<Layer, LayerType> layers_types;
  for (const auto &[layer, groups] : topology.groups_per_layer)
  {
    std::ignore = groups;
    layers_types.emplace(layer, check_layer_type(topology, layer));
  }
  return layers_types;
}
} // namespace Model
