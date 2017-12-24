#pragma once

#include "source_stream.h"

#include <random>

class PoissonSourceStream : public SourceStream
{
  Intensity intensity_;
  Size load_size_;

  std::exponential_distribution<long double> exponential{ts::get(intensity_)};

  EventPtr produce_load(Time time);
public:
  PoissonSourceStream(const SourceName &name, Intensity intensity, Size load_size);

  void init() override;
  void notify_on_send(const LoadSendEvent *event) override;

  Size get_load_size() override;
  Intensity get_intensity() override;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PoissonSourceStream &source);
