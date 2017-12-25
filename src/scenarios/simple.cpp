
#include "simple.h"

#include "topology.h"
#include "calculation.h"
#include "logger.h"
#include "group.h"

#include "source_stream/poisson.h"
#include "source_stream/engset.h"

SimulationSettings erlang_model(const Intensity lambda, const Capacity V)
{
  auto serve_intensity = Intensity(1.0L);

  SimulationSettings sim_settings{"Erlang model"};

  const auto A = lambda / serve_intensity;
  sim_settings.do_before = [=]() {
    print("[Erlang] P_loss = P_block = E_V(A) = {}\n",
          erlang_pk(A, ts::get(V), ts::get(V)));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;

  SourceName s1{"SPo1"};
  topology.add_source(
      std::make_unique<PoissonSourceStream>(s1, lambda, Size(1)));

  GroupName g1{"G1"};
  topology.add_group(std::make_unique<Group>(g1, V));
  topology.attach_source_to_group(s1, g1);

  topology.add_traffic_class(s1, g1, serve_intensity);

  return sim_settings;
}

SimulationSettings
engset_model(const Intensity gamma, const Capacity V, const Count N)
{ // Engset model

  SimulationSettings sim_settings{"Engset model"};

  const auto serve_intensity = Intensity(1.0L);
  const auto alpha = gamma / serve_intensity;

  sim_settings.do_before = [=]() {
    print("[Engset] P_block = E(alfa, V, N) = {}\n",
          engset_pi(alpha, ts::get(V), ts::get(N), ts::get(V)));
    print("[Engset] P_loss = B(alpha, V, N) = E(alfa, V, N-1) = {}\n",
          engset_pi(alpha, ts::get(V), ts::get(N) - 1, ts::get(V)));
  };
  sim_settings.do_after = sim_settings.do_before;

  auto &topology = sim_settings.topology;
  GroupName g1{"G1"};
  SourceName s1{"SEn1"};
  topology.add_group(std::make_unique<Group>(g1, V));

  topology.add_source(
      std::make_unique<EngsetSourceStream>(s1, gamma, N, Size(1)));
  topology.attach_source_to_group(s1, g1);
  topology.add_traffic_class(s1, g1, serve_intensity);

  return sim_settings;
}
