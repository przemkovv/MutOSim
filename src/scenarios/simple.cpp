
#include "simple.h"

#include "calculation.h"
#include "group.h"
#include "logger.h"
#include "topology.h"

#include "source_stream/engset.h"
#include "source_stream/poisson.h"
#include "source_stream/pascal.h"

//----------------------------------------------------------------------
SimulationSettings erlang_model(const Intensity lambda, const Capacity V)
{
  const auto serve_intensity = Intensity(1.0L);
  const auto A = lambda / serve_intensity;
  const auto size = Size(1);

  SimulationSettings sim_settings{"Erlang model"};

  sim_settings.do_before = [=]() {
    print("[Erlang] P_loss = P_block = E_V(A) = {}\n",
          erlang_pk(A, ts::get(V), ts::get(V)));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;

  auto &tc = topology.add_traffic_class(lambda, serve_intensity, size);

  SourceName s1{"SPo1"};
  topology.add_source(std::make_unique<PoissonSourceStream>(s1, tc));

  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}

SimulationSettings
engset_model(const Intensity gamma, const Capacity V, const Count N)
{ // Engset model

  SimulationSettings sim_settings{"Engset model"};

  const auto serve_intensity = Intensity(1.0L);
  const auto alpha = gamma / serve_intensity;
  const auto size = Size(1);

  sim_settings.do_before = [=]() {
    print("[Engset] P_block = E(alfa, V, N) = {}\n",
          engset_pi(alpha, ts::get(V), ts::get(N), ts::get(V)));
    print("[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
          engset_pi(alpha, ts::get(V), ts::get(N) - 1, ts::get(V)));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;
  auto &tc = topology.add_traffic_class(gamma, serve_intensity, size);
  GroupName g1{"G1"};
  SourceName s1{"SEn1"};
  topology.add_group(std::make_unique<Group>(g1, V));

  topology.add_source(std::make_unique<EngsetSourceStream>(s1, tc, N));
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}

//----------------------------------------------------------------------
SimulationSettings poisson_streams(std::vector<Intensity> As,
                                   std::vector<Size> sizes,
                                   Capacity primary_V)
{
  auto serve_intensity = Intensity(1.0L);
  SimulationSettings sim_settings{"Single group with Poisson streams"};

  auto &topology = sim_settings.topology;

  std::vector<SourceName> source_names;

  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Group>(g1, primary_V));

  for (auto source_number = 0u; source_number < As.size(); ++source_number) {
    SourceName sn{fmt::format("S{}", source_number)};
    Size z = sizes[source_number];
    auto &tc =
        topology.add_traffic_class(As[source_number], serve_intensity, z);
    topology.add_source(std::make_unique<PoissonSourceStream>(sn, tc));

    topology.attach_source_to_group(sn, g1);
  }

  return sim_settings;
}

//----------------------------------------------------------------------

SimulationSettings pascal_model(Intensity gamma, Capacity V, Count N)
{
  const auto serve_intensity = Intensity(1.0L);
  // const auto A = gamma / serve_intensity;
  const auto size = Size(1);

  SimulationSettings sim_settings{"Pascal model"};

  auto &topology = sim_settings.topology;

  auto &tc = topology.add_traffic_class(gamma, serve_intensity, size);

  SourceName s1{"SPa1"};
  topology.add_source(std::make_unique<PascalSourceStream>(s1, tc, N));

  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.attach_source_to_group(s1, g1);

  return sim_settings;
}
