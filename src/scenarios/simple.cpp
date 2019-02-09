
#include "simple.h"

#include "calculation.h"
#include "logger.h"
#include "simulation/group.h"
#include "simulation/source_stream/engset.h"
#include "simulation/source_stream/pascal.h"
#include "simulation/source_stream/poisson.h"
#include "topology.h"
#include "types/types_format.h"


//----------------------------------------------------------------------
ScenarioSettings
erlang_model(const Intensity lambda, const Capacity V)
{
  const auto serve_intensity = Intensity(1.0L);
  const auto A = lambda / serve_intensity;
  const auto size = Size(1);

  auto name =
      fmt::format("Erlang model. lambda={},  mu={}, V={} ", lambda, serve_intensity, V);
  ScenarioSettings sim_settings{name};

  sim_settings.do_before = [=]() {
    print(
        "[Erlang] P_loss = P_block = E_V(A) = {}\n",
        erlang_pk_distribution(get(A), ts::get(V), ts::get(V)));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;

  auto &tc = topology.add_traffic_class(lambda, serve_intensity, size);

  SourceName s1{"SPo1"};
  topology.add_source(std::make_unique<Simulation::PoissonSourceStream>(s1, tc));

  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Simulation::Group>(g1, V));
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}

ScenarioSettings
engset_model(const Intensity lambda, const Capacity V, const Count N)
{ // Engset model

  const auto serve_intensity = Intensity(1.0L);
  const auto gamma = lambda / N;
  const auto alpha = gamma / serve_intensity;
  const auto size = Size(1);

  auto name = fmt::format(
      "Engset model. alpha={}, lambda={}, gamma={}, mu={}, V={}, N={} ",
      alpha,
      lambda,
      gamma,
      serve_intensity,
      V,
      N);
  ScenarioSettings sim_settings{name};

  sim_settings.do_before = [=]() {
    print(
        "[Engset] P_block = E(alfa, V, N) = {}\n",
        engset_pi(get(alpha), ts::get(V), ts::get(N), ts::get(V)));
    print(
        "[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
        engset_pi(get(alpha), ts::get(V), ts::get(N) - 1, ts::get(V)));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;
  auto &tc = topology.add_traffic_class(lambda, serve_intensity, size);
  GroupName g1{"G1"};
  SourceName s1{"SEn1"};
  topology.add_group(std::make_unique<Simulation::Group>(g1, V));

  topology.add_source(std::make_unique<Simulation::EngsetSourceStream>(s1, tc, N));
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}

//----------------------------------------------------------------------
ScenarioSettings
poisson_streams(std::vector<Intensity> As, std::vector<Size> sizes, Capacity primary_V)
{
  auto serve_intensity = Intensity(1.0L);
  ScenarioSettings sim_settings{"Single group with Poisson streams"};

  auto &topology = sim_settings.topology;

  std::vector<SourceName> source_names;

  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Simulation::Group>(g1, primary_V));

  for (auto source_number = 0u; source_number < As.size(); ++source_number) {
    SourceName sn{fmt::format("S{}", source_number)};
    Size z = sizes[source_number];
    auto &tc = topology.add_traffic_class(As[source_number], serve_intensity, z);
    topology.add_source(std::make_unique<Simulation::PoissonSourceStream>(sn, tc));

    topology.attach_source_to_group(sn, g1);
  }

  return sim_settings;
}

//----------------------------------------------------------------------

ScenarioSettings
pascal_source_model(Intensity lambda, Capacity V, Count S)
{ // Pascal source

  auto serve_intensity = Intensity(1.0L);
  auto gamma = lambda / S;
  auto size = Size(1);

  auto name = fmt::format(
      "Pascal source. lambda={}, gamma={}, mu={}, V={}, S={} ",
      lambda,
      gamma,
      serve_intensity,
      V,
      S);
  ScenarioSettings sim_settings{name};

  // const auto lambda = Intensity(5);
  // const auto S = Size(5);
  // const auto gamma = lambda / S;
  // const auto micro = Intensity(1.0);
  // const auto V = Size(7);
  // const auto alpha = gamma / serve_intensity;

  auto &topology = sim_settings.topology;
  auto &tc = topology.add_traffic_class(lambda, serve_intensity, size);
  GroupName g1{"G1"};
  SourceName s1{"SPa1"};
  topology.add_group(std::make_unique<Simulation::Group>(g1, V));

  topology.add_source(std::make_unique<Simulation::PascalSourceStream>(s1, tc, S));
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}
