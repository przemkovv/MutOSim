
#include "analytical.h"

#include "logger.h"
#include "overflow_far.h"
#include "traffic_class.h"
#include "types.h"

#include <algorithm>
#include <numeric>

namespace Model
{
void analytical_computations()
{
  std::vector<TrafficClass> traffic_classes;

  traffic_classes.emplace_back(
      TrafficClass{TrafficClassId{1}, Intensity{10.0}, Intensity{1.0}, Size{1}});
  traffic_classes.emplace_back(
      TrafficClass{TrafficClassId{2}, Intensity{10.0}, Intensity{1.0}, Size{4}});
  traffic_classes.emplace_back(
      TrafficClass{TrafficClassId{3}, Intensity{10.0}, Intensity{1.0}, Size{9}});

  for (const auto &traffic_class : traffic_classes) {
    println("{}", traffic_class);
  }

  std::vector<std::vector<RequestStream>> request_streams_groups;
  request_streams_groups.emplace_back(
      KaufmanRobertsBlockingProbability(traffic_classes, Capacity{60}));
  request_streams_groups.emplace_back(
      KaufmanRobertsBlockingProbability(traffic_classes, Capacity{50}));
  request_streams_groups.emplace_back(
      KaufmanRobertsBlockingProbability(traffic_classes, Capacity{40}));

  println("Blocking probability for primary groups from Kaufman-Roberts:\n{}",
          request_streams_groups);

  // TODO(PW): aggregate over size and groups mean and variance

  // Formulas 3.17 and 3.18
  std::unordered_map<TrafficClassId, RequestStreamProperties> request_streams_properties;
  for (const auto &request_streams : request_streams_groups) {
    for (const auto &request_stream : request_streams) {
      auto &properties = request_streams_properties[request_stream.tc.id];
      properties.mean += request_stream.mean;
      properties.variance_sq += request_stream.variance_sq;
      properties.tc = request_stream.tc;
    }
  }
  std::for_each(begin(request_streams_properties), end(request_streams_properties),
                [](auto &properties) {
                  properties.second.peakness =
                      properties.second.variance_sq / properties.second.mean;
                });

  println("Accumulated by class");
  for (const auto &[tc_id, properties] : request_streams_properties) {
    std::ignore = tc_id;
    println("{}", properties);
  }

  // Formula 3.20
  double inv_sum =
      1.0 / std::accumulate(begin(request_streams_properties),
                            end(request_streams_properties), 0.0,
                            [](auto x, const auto &rs) {
                              return x + rs.second.mean * get(rs.second.tc.size);
                            });

  // Formula 3.19
  double peakness = std::accumulate(
      begin(request_streams_properties), end(request_streams_properties), 0.0,
      [inv_sum](auto x, const auto &rs) {
        return x + rs.second.variance_sq * get(rs.second.tc.size) * inv_sum;
      });

  println("Peakness: {}", peakness);

  std::vector<RequestStreamProperties> request_streams_properties_vec;

  std::transform(begin(request_streams_properties), end(request_streams_properties),
                 std::back_inserter(request_streams_properties_vec),
                 [](const auto &p) { return p.second; });


  println("Properties:\n{}", request_streams_properties_vec);

  auto alternativeGroup = KaufmanRobertsBlockingProbability(
      request_streams_properties_vec, Capacity{80}, peakness);

  println("Alternative group:");
  println("{}", alternativeGroup);
}

} // namespace Model
