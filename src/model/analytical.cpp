
#include "analytical.h"

#include "logger.h"
#include "model/group.h"
#include "overflow_far.h"
#include "scenario_settings.h"
#include "simulation/group.h"
#include "simulation/source_stream/source_stream.h"
#include "traffic_class.h"

#include <map>

namespace Model
{
void
analytical_computations_hardcoded()
{
  std::vector<IncomingRequestStream> traffic_classes1;
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{1}, Intensity{20.0L}, Intensity{1.0L}, Size{1}, {}});
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{2}, Intensity{10.0L}, Intensity{1.0L}, Size{2}, {}});
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{3}, Intensity{3.33333L}, Intensity{1.0L}, Size{6}, {}});
  Model::Group g1{Capacity{60}};
  Model::Group g2{Capacity{60}};
  Model::Group g3{Capacity{60}};
  g1.add_incoming_request_streams(traffic_classes1);
  g2.add_incoming_request_streams(traffic_classes1);
  g3.add_incoming_request_streams(traffic_classes1);

  Model::Group g0{Capacity{42}};
  g0.add_incoming_request_streams(g1.get_outgoing_request_streams());
  g0.add_incoming_request_streams(g2.get_outgoing_request_streams());
  g0.add_incoming_request_streams(g3.get_outgoing_request_streams());

  println("{}", g0.get_outgoing_request_streams());
}
void
analytical_computations(ScenarioSettings &scenario_settings)
{
  const auto &topology = scenario_settings.topology;

  using Groups = std::map<GroupName, Model::Group>;
  using GroupsPtr = std::map<GroupName, Model::Group *>;
  Groups groups;
  std::map<Layer, GroupsPtr> groups_layers;

  for (const auto &[group_name, group] : topology.groups) {
    ASSERT(
        group->next_groups().size() <= 1,
        "The current model doesn't support forwarding traffic to more than one next "
        "groups.");
    auto [group_it, inserted] =
        groups.emplace(group_name, Model::Group{group->capacity()});

    for (const auto &next_group : group->next_groups()) {
      group_it->second.add_next_group(next_group->name());
    }
    groups_layers[group->layer()].emplace(group_name, &group_it->second);
  }

  for (const auto &[source_name, source_stream] : topology.sources) {
    const auto &target_group = source_stream->get_target_group().name();
    groups.at(target_group)
        .add_incoming_request_stream(IncomingRequestStream{source_stream->tc_});
  }

  for (const auto &[layer, groups_ptrs] : groups_layers) {
    std::ignore = layer;
    for (const auto &[group_name, group_ptr] : groups_ptrs) {
      std::ignore = group_name;
      for (const auto &next_group_name : group_ptr->next_groups()) {
        groups.at(next_group_name)
            .add_incoming_request_streams(group_ptr->get_outgoing_request_streams());
      }
    }
  }

  auto &stats = scenario_settings.stats;
  for (const auto &[layer, groups_ptrs] : groups_layers) {
    for (const auto &[group_name, group_ptr] : groups_ptrs) {
      auto &group_stats = stats[get(group_name)];
      debug_println("Layer {}, Group {}: ", layer, group_name);
      debug_println("{}", group_ptr->get_outgoing_request_streams());
      for (const auto &out_stream : group_ptr->get_outgoing_request_streams()) {
        auto &j_tc = group_stats[std::to_string(get(out_stream.tc.id))];
        j_tc["P_block"] = get(out_stream.blocking_probability);
      }
    }
  }
}

} // namespace Model
