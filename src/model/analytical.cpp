
#include "analytical.h"

#include "logger.h"
#include "model/group.h"
#include "overflow_far.h"
#include "traffic_class.h"

namespace Model
{
void
analytical_computations(const ScenarioSettings & /* scenario_settings */)
{
  std::vector<IncomingRequestStream> traffic_classes1;
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{1}, Intensity{20.0L}, Intensity{1.0L}, Size{1}, {}});
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{2}, Intensity{10.0L}, Intensity{1.0L}, Size{2}, {}});
  traffic_classes1.emplace_back(
      TrafficClass{TrafficClassId{3}, Intensity{3.333L}, Intensity{1.0L}, Size{6}, {}});
  Model::Group g1{Capacity{60}};
  Model::Group g2{Capacity{60}};
  Model::Group g3{Capacity{60}};
  g1.add_incoming_request_stream(traffic_classes1);
  g2.add_incoming_request_stream(traffic_classes1);
  g3.add_incoming_request_stream(traffic_classes1);

  Model::Group g0{Capacity{42}};
  g0.add_incoming_request_stream(g1.get_outgoing_request_streams());
  g0.add_incoming_request_stream(g2.get_outgoing_request_streams());
  g0.add_incoming_request_stream(g3.get_outgoing_request_streams());

  println("{}", g0.get_outgoing_request_streams());
}

} // namespace Model
