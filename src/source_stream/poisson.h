#pragma once

#include "source_stream.h"

#include <random>

class PoissonSourceStream : public SourceStream
{
  std::exponential_distribution<time_type> exponential{ts::get(tc_.source_intensity)};

  EventPtr produce_load(Time time);

public:
  PoissonSourceStream(const SourceName &name, const TrafficClass &tc);
  void reset() override { SourceStream::reset(); }

  void init() override;
  void notify_on_service_start(const LoadServiceRequestEvent *event) override;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PoissonSourceStream &source);
