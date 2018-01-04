
#include "single_overflow.h"

#include "calculation.h"
#include "group.h"
#include "logger.h"
#include "topology.h"

#include "source_stream/engset.h"
#include "source_stream/poisson.h"

//----------------------------------------------------------------------
SimulationSettings single_overflow_poisson(const Intensity lambda, const Capacity V)
{
  auto serve_intensity = Intensity(1.0L);
  auto size = Size(1);
  SimulationSettings sim_settings{"Single overflow Poisson"};

  auto &topology = sim_settings.topology;
  auto &tc1 = topology.add_traffic_class(lambda, serve_intensity, size);
  GroupName g1{"G1"};
  GroupName g2{"G2"};
  SourceName s1{"Spo1"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.add_group(std::make_unique<Group>(g2, V));
  topology.add_source(std::make_unique<PoissonSourceStream>(s1, tc1));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}

//----------------------------------------------------------------------
SimulationSettings
single_overflow_poisson(Intensity A,
                        std::vector<Capacity> primary_Vs,
                        std::vector<std::vector<Size>> size_per_class_per_source,
                        Capacity secondary_V)
{
  auto serve_intensity = Intensity(1.0L);
  SimulationSettings sim_settings{"Single overflow Poisson"};

  auto &topology = sim_settings.topology;

  std::vector<GroupName> group_names;
  std::vector<SourceName> source_names;

  for (auto group_number = 0u; group_number < primary_Vs.size(); ++group_number) {
    GroupName gn{fmt::format("G{}", group_number)};
    topology.add_group(std::make_unique<Group>(gn, primary_Vs[group_number]));
    group_names.emplace_back(std::move(gn));
  }

  GroupName gvo{"GVo"};
  topology.add_group(std::make_unique<Group>(gvo, secondary_V));

  for (auto source_number = 0u; source_number < size_per_class_per_source.size();
       ++source_number) {
    for (auto class_number = 0u;
         class_number < size_per_class_per_source[source_number].size(); ++class_number) {
      Size t = size_per_class_per_source[source_number][class_number];
      auto &tc = topology.add_traffic_class(A / t, serve_intensity, t);

      SourceName sn{fmt::format("S{}{}", class_number, source_number)};
      topology.add_source(std::make_unique<PoissonSourceStream>(sn, tc));

      topology.attach_source_to_group(sn, group_names[source_number]);
    }
  }

  for (const auto &group_name : group_names) {
    topology.connect_groups(group_name, gvo);
  }

  return sim_settings;
}

//----------------------------------------------------------------------
SimulationSettings
single_overflow_engset(const Intensity gamma, const Capacity V, const Count N)
{
  auto serve_intensity = Intensity(1.0L);
  auto size = Size(1);
  SimulationSettings sim_settings{"Single overflow Engset"};

  auto &topology = sim_settings.topology;
  auto &tc = topology.add_traffic_class(gamma, serve_intensity, size);
  GroupName g1{"G1"};
  GroupName g2{"G2"};
  SourceName s1{"Spo1"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.add_group(std::make_unique<Group>(g2, V));
  topology.add_source(std::make_unique<EngsetSourceStream>(s1, tc, N));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}
//----------------------------------------------------------------------

SimulationSettings multiple_sources_single_overflow()
{
  auto serve_intensity = Intensity(1.0L);
  const auto lambda = Intensity(3);
  const auto N = Count(2);
  // const auto gamma = lambda / N;
  const auto V = Capacity(2);
  const auto size1 = Size(1);
  const auto size2 = Size(2);
  // const auto alpha = gamma / micro;

  SimulationSettings sim_settings{"Multiple sources - Single overflow"};

  auto &topology = sim_settings.topology;

  auto &tc1 = topology.add_traffic_class(lambda, serve_intensity, size1);
  auto &tc2 = topology.add_traffic_class(lambda, serve_intensity, size2);

  GroupName g1{"G1"};
  GroupName g2{"G2"};
  SourceName s1{"SPo1"};
  SourceName s2{"SEn2"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.add_group(std::make_unique<Group>(g2, V));
  topology.add_source(std::make_unique<PoissonSourceStream>(s1, tc1));
  topology.add_source(std::make_unique<EngsetSourceStream>(s2, tc2, N));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);
  topology.attach_source_to_group(s2, g1);

  return sim_settings;
}
//----------------------------------------------------------------------
