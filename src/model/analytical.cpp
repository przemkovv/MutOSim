
#include "analytical.h"

#include "logger.h"
#include "overflow_far.h"
#include "traffic_class.h"
#include "types.h"

#include <numeric>

namespace Model
{
void analytical_computations()
{
  std::vector<TrafficClass> traffic_classes;

  traffic_classes.emplace_back(
      TrafficClass{TrafficClassId{1}, Intensity{1.0}, Intensity{1.0}, Size{1}});
  traffic_classes.emplace_back(
      TrafficClass{TrafficClassId{2}, Intensity{2.0}, Intensity{1.0}, Size{4}});
  traffic_classes.emplace_back(
      TrafficClass{TrafficClassId{3}, Intensity{1.0}, Intensity{1.0}, Size{9}});

  for (const auto &traffic_class : traffic_classes) {
    println("{}", traffic_class);
  }

  auto request_streams = KaufmanRobertsBlockingProbability(traffic_classes, Capacity{10});

  println("Blocking probability for primary groups from Kaufman-Roberts:\n{}",
          request_streams);

  // TODO(PW): aggregate over size and groups mean and variance

  double inv_sum = 1.0 / std::accumulate(begin(request_streams), end(request_streams),
                                         0.0, [](auto x, const RequestStream &rs) {
                                           return x + rs.mean * get(rs.tc.size);
                                         });

  double peakness = std::accumulate(begin(request_streams), end(request_streams), 0.0,
                                    [inv_sum](auto x, const RequestStream &rs) {
                                      return x + rs.variance_sq *
                                                     get(rs.tc.size) * inv_sum;
                                    });

  println("InvSum: {}", inv_sum);
  println("Peakness: {}", peakness);
}

} // namespace Model
