#pragma once

#include "source_stream.h"

#include <fmt/format.h>
#include <random>

namespace Simulation
{
class EngsetSourceStream : public SourceStream
{
  Count sources_number_;
  Count active_sources_{0};

  std::exponential_distribution<time_type> exponential{
      ts::get(tc_.source_intensity / sources_number_)};

  std::unique_ptr<ProduceServiceRequestEvent> create_produce_service_request(Time time);

  template <typename T, typename Char, typename Enable>
  friend struct fmt::formatter;

  EventPtr create_request(Time time);

public:
  void init() override;
  void notify_on_request_service_end(const LoadServiceEndEvent *event) override;
  void notify_on_produce(const ProduceServiceRequestEvent *event) override;
  void notify_on_request_accept(const LoadServiceRequestEvent *event) override;
  void notify_on_request_drop(const LoadServiceRequestEvent *event) override;

  EngsetSourceStream(
      const SourceName &name, const TrafficClass &tc, Count sources_number);
};

} // namespace Simulation

