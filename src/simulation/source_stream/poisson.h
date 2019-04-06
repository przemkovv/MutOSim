#pragma once

#include "source_stream.h"

#include <random>

namespace Simulation {
class PoissonSourceStream : public SourceStream

{
  std::exponential_distribution<time_type<>> exponential{ts::get(tc_.source_intensity)};

  EventPtr create_request(Time time);

public:
  PoissonSourceStream(const SourceName &name, const TrafficClass &tc);

  void init() override;
  void notify_on_request_service_start(const LoadServiceRequestEvent *event) override;
};

} // namespace Simulation
