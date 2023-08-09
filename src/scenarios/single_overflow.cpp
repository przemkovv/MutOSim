
#include "single_overflow.h"

#include "calculation.h"
#include "logger.h"
#include "simple.h"
#include "simulation/group.h"
#include "simulation/source_stream/engset.h"
#include "simulation/source_stream/poisson.h"
#include "topology.h"
#include "types/types_format.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

using Simulation::Capacity;
using Simulation::Count;
using Simulation::Intensity;
using Simulation::Size;

//----------------------------------------------------------------------
ScenarioSettings
single_overflow_poisson(const Intensity lambda, const Capacity V)
{
  auto             serve_intensity = Intensity(1.0L);
  auto             size = Size(1);
  ScenarioSettings sim_settings{"Single overflow Poisson"};

  auto      &topology = sim_settings.topology;
  auto      &tc1 = topology.add_traffic_class(lambda, serve_intensity, size);
  GroupName  g1{"G1"};
  GroupName  g2{"G2"};
  SourceName s1{"Spo1"};
  topology.add_group(std::make_unique<Simulation::Group>(g1, V));
  topology.add_group(std::make_unique<Simulation::Group>(g2, V));
  topology.add_source(
      std::make_unique<Simulation::PoissonSourceStream>(s1, tc1));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}

//----------------------------------------------------------------------
ScenarioSettings
single_overflow_poisson(
    Intensity                      A,
    std::vector<Capacity>          primary_Vs,
    std::vector<std::vector<Size>> size_per_class_per_source,
    Capacity                       secondary_V)
{
  auto             serve_intensity = Intensity(1.0L);
  ScenarioSettings sim_settings{"Single overflow Poisson"};

  auto &topology = sim_settings.topology;

  std::vector<GroupName>  group_names;
  std::vector<SourceName> source_names;

  for (auto group_number = 0u; group_number < primary_Vs.size(); ++group_number)
  {
    GroupName gn{fmt::format("G{}", group_number)};
    topology.add_group(
        std::make_unique<Simulation::Group>(gn, primary_Vs[group_number]));
    group_names.emplace_back(std::move(gn));
  }

  GroupName gvo{"GVo"};
  topology.add_group(std::make_unique<Simulation::Group>(gvo, secondary_V));

  for (auto source_number = 0u;
       source_number < size_per_class_per_source.size();
       ++source_number)
  {
    for (auto class_number = 0u;
         class_number < size_per_class_per_source[source_number].size();
         ++class_number)
    {
      Size  t = size_per_class_per_source[source_number][class_number];
      auto &tc = topology.add_traffic_class(A / t, serve_intensity, t);

      SourceName sn{fmt::format("S{}{}", class_number, source_number)};
      topology.add_source(
          std::make_unique<Simulation::PoissonSourceStream>(sn, tc));

      topology.attach_source_to_group(sn, group_names[source_number]);
    }
  }

  for (const auto &group_name : group_names)
  {
    topology.connect_groups(group_name, gvo);
  }

  return sim_settings;
}

//----------------------------------------------------------------------
ScenarioSettings
single_overflow_engset(const Intensity gamma, const Capacity V, const Count N)
{
  auto             serve_intensity = Intensity(1.0L);
  auto             size = Size(1);
  ScenarioSettings sim_settings{"Single overflow Engset"};

  auto      &topology = sim_settings.topology;
  auto      &tc = topology.add_traffic_class(gamma, serve_intensity, size);
  GroupName  g1{"G1"};
  GroupName  g2{"G2"};
  SourceName s1{"Spo1"};
  topology.add_group(std::make_unique<Simulation::Group>(g1, V));
  topology.add_group(std::make_unique<Simulation::Group>(g2, V));
  topology.add_source(
      std::make_unique<Simulation::EngsetSourceStream>(s1, tc, N));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}
//----------------------------------------------------------------------

ScenarioSettings
multiple_sources_single_overflow()
{
  auto       serve_intensity = Intensity(1.0L);
  const auto lambda = Intensity(3);
  const auto N = Count(2);
  // const auto gamma = lambda / N;
  const auto V = Capacity(2);
  const auto size1 = Size(1);
  const auto size2 = Size(2);
  // const auto alpha = gamma / micro;

  ScenarioSettings sim_settings{"Multiple sources - Single overflow"};

  auto &topology = sim_settings.topology;

  auto &tc1 = topology.add_traffic_class(lambda, serve_intensity, size1);
  auto &tc2 = topology.add_traffic_class(lambda, serve_intensity, size2);

  GroupName  g1{"G1"};
  GroupName  g2{"G2"};
  SourceName s1{"SPo1"};
  SourceName s2{"SEn2"};
  topology.add_group(std::make_unique<Simulation::Group>(g1, V));
  topology.add_group(std::make_unique<Simulation::Group>(g2, V));
  topology.add_source(
      std::make_unique<Simulation::PoissonSourceStream>(s1, tc1));
  topology.add_source(
      std::make_unique<Simulation::EngsetSourceStream>(s2, tc2, N));

  topology.connect_groups(g1, g2);
  topology.attach_source_to_group(s1, g1);
  topology.attach_source_to_group(s2, g1);

  return sim_settings;
}
//----------------------------------------------------------------------
//
void
prepare_custom_scenarios(
    std::vector<ScenarioSettings> &scenarios,
    const CLIOptions              &cli)
{
  if ((false))
  {
    for (auto A = cli.A_start; A < cli.A_stop; A += cli.A_step)
    {
      std::vector<Size>    sizes{Size{1}, Size{1}, Size{3}, Size{3}};
      std::vector<int64_t> ratios{1, 1, 1, 1};
      auto ratios_sum = std::accumulate(begin(ratios), end(ratios), 0ll);
      std::vector<long double> ratios_d(begin(ratios), end(ratios));
      for_each(begin(ratios_d), end(ratios_d), [ratios_sum](auto &x) {
        x /= ratios_sum;
      });

      auto V = Capacity{30};

      std::vector<Intensity> intensities{sizes.size()};
      for (auto i = 0u; i < sizes.size(); ++i)
      {
        intensities[i] =
            Intensity{get(A) * get(V) / get(sizes[i]) * ratios_d[i]};
      }
      auto &scenario =
          scenarios.emplace_back(poisson_streams(intensities, sizes, V));
      scenario.name += fmt::format(" A={}", A);
    }
  }

  if ((false))
  {
    scenarios.emplace_back(single_overflow_poisson(
        Intensity(24.0L),
        {Capacity{60}, Capacity{60}, Capacity{60}},
        {{Size{1}, Size{2}, Size{6}},
         {Size{1}, Size{2}, Size{6}},
         {Size{1}, Size{2}, Size{6}}},
        Capacity{42}));
  }

  if ((false))
  {
    scenarios.emplace_back(
        single_overflow_poisson(Intensity(2.0L), Capacity(2)));
    scenarios.emplace_back(
        single_overflow_poisson(Intensity(4.0L), Capacity(2)));
    scenarios.emplace_back(
        single_overflow_poisson(Intensity(6.0L), Capacity(2)));
  }

  if ((false))
  {
    scenarios.emplace_back(
        pascal_source_model(Intensity(1.0L), Capacity(1), Count(1)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(1.0L), Capacity(2), Count(1)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(1.0L), Capacity(1), Count(2)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(1.0L), Capacity(1), Count(1)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(1.0L), Capacity(2), Count(1)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(1.0L), Capacity(3), Count(1)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(1.0L), Capacity(4), Count(1)));
  }

  if ((false))
  {
    scenarios.emplace_back(
        pascal_source_model(Intensity(15.0L), Capacity(30), Count(20)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(30.0L), Capacity(30), Count(20)));
    scenarios.emplace_back(
        pascal_source_model(Intensity(45.0L), Capacity(30), Count(20)));
  }

  if ((false))
  {
    scenarios.emplace_back(erlang_model(Intensity(15.0L), Capacity(30)));
    scenarios.emplace_back(erlang_model(Intensity(30.0L), Capacity(30)));
    scenarios.emplace_back(erlang_model(Intensity(45.0L), Capacity(30)));
  }

  if ((false))
  {
    scenarios.emplace_back(
        engset_model(Intensity(10.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(
        engset2_model(Intensity(10.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(
        engset_model(Intensity(20.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(
        engset2_model(Intensity(20.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(
        engset_model(Intensity(30.0L), Capacity(20), Count(40)));
    scenarios.emplace_back(
        engset2_model(Intensity(30.0L), Capacity(20), Count(40)));
  }
}
