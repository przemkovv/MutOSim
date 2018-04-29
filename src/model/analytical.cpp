
#include "analytical.h"

#include "logger.h"
#include "overflow_far.h"
#include "traffic_class.h"

#include <algorithm>
#include <boost/variant/variant.hpp>
#include <numeric>

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

  std::vector<IncomingRequestStream> traffic_classes2;
  traffic_classes2.emplace_back(
      TrafficClass{TrafficClassId{1}, Intensity{20.0L}, Intensity{1.0L}, Size{1}, {}});
  traffic_classes2.emplace_back(
      TrafficClass{TrafficClassId{2}, Intensity{10.0L}, Intensity{1.0L}, Size{2}, {}});
  traffic_classes2.emplace_back(
      TrafficClass{TrafficClassId{3}, Intensity{3.333L}, Intensity{1.0L}, Size{6}, {}});

  std::vector<IncomingRequestStream> traffic_classes3;
  traffic_classes3.emplace_back(
      TrafficClass{TrafficClassId{1}, Intensity{20.0L}, Intensity{1.0L}, Size{1}, {}});
  traffic_classes3.emplace_back(
      TrafficClass{TrafficClassId{2}, Intensity{10.0L}, Intensity{1.0L}, Size{2}, {}});
  traffic_classes3.emplace_back(
      TrafficClass{TrafficClassId{3}, Intensity{3.333L}, Intensity{1.0L}, Size{6}, {}});

  std::vector<std::vector<OutgoingRequestStream>> request_streams_per_group;
  request_streams_per_group.emplace_back(
      KaufmanRobertsBlockingProbability(traffic_classes1, Capacity{60}));
  request_streams_per_group.emplace_back(
      KaufmanRobertsBlockingProbability(traffic_classes2, Capacity{60}));
  request_streams_per_group.emplace_back(
      KaufmanRobertsBlockingProbability(traffic_classes3, Capacity{60}));

  println(
      "Blocking probability for primary groups from Kaufman-Roberts:\n{}",
      request_streams_per_group);

  // Formulas 3.17 and 3.18
  auto overflowing_request_streams =
      convert_to_overflowing_streams(request_streams_per_group);
  overflowing_request_streams.emplace_back(
      TrafficClass{TrafficClassId{4}, Intensity{20.0L}, Intensity{1.0L}, Size{1}, {}});

  // Formula 3.19
  auto peakness = compute_collective_peakness(overflowing_request_streams);

  println("Peakness: {}", peakness);
  println("Properties:\n{}", overflowing_request_streams);

  auto alternativeGroupStreams = KaufmanRobertsBlockingProbability(
      overflowing_request_streams, Capacity{Capacity{42} / peakness});

  println("Alternative group:");
  println("{}", alternativeGroupStreams);
}

} // namespace Model
