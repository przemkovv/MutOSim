#pragma once

#include "source_stream.h"

#include <random>

class EngsetSourceStream : public SourceStream
{
  Count sources_number_;
  Count active_sources_{0};

  std::exponential_distribution<time_type> exponential{ts::get(tc_.source_intensity)};

  friend void format_arg(fmt::BasicFormatter<char> &f,
                         const char *&format_str,
                         const EngsetSourceStream &source);

  std::unique_ptr<ProduceServiceRequestEvent> create_produce_service_request(Time time);

  EventPtr produce_load(Time time);

public:
  void init() override;
  void notify_on_service_end(const LoadServiceEndEvent *event) override;
  void notify_on_produce(const ProduceServiceRequestEvent *event) override;

  EngsetSourceStream(const SourceName &name,
                     const TrafficClass &tc,
                     Count sources_number);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const EngsetSourceStream &source);
