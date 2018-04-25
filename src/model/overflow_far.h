#pragma once

#include "traffic_class.h"
#include "types.h"

#include <vector>

namespace Model
{
struct RequestStream {
  float blocking_probability;
  TrafficClass tc;
  Intensity intensity = Intensity{tc.source_intensity / tc.serve_intensity};
};

float KaufmanRoberts(std::vector<RequestStream> &request_streams, Capacity V, Capacity n);

class OverflowFar
{
  struct PrimaryResource {
    Capacity capacity;
    std::vector<RequestStream> request_streams;
  };

  std::vector<PrimaryResource> primary_resources_;
};

} // namespace Model
