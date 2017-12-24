#pragma once

#include "source_stream.h"

#include <random>

class PoissonSourceStream : public SourceStream
{
  Intensity intensity_;
  Size load_size_;

  std::exponential_distribution<long double> exponential{intensity_};

  EventPtr produce_load(Time time);
public:
  PoissonSourceStream(const SourceName &name, Intensity intensity, Size load_size);

  void init() override;
  void notify_on_send(const LoadSendEvent *event) override;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PoissonSourceStream &source);
