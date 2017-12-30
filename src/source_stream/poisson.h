#pragma once

#include "source_stream.h"

#include <random>

class PoissonSourceStream : public SourceStream
{
  std::exponential_distribution<long double> exponential{ts::get(tc_.source_intensity)};

  EventPtr produce_load(Time time);
public:
  PoissonSourceStream(const SourceName &name, const TrafficClass& tc);

  void init() override;
  void notify_on_send(const LoadSendEvent *event) override;

};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PoissonSourceStream &source);
