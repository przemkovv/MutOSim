#pragma once

#include "source_stream.h"

#include <random>

class PoissonSourceStream : public SourceStream
{
  double intensity_;
  Size load_size_;

  std::uniform_real_distribution<> uniform{0.0, 1.0};
  std::exponential_distribution<> exponential{intensity_};

public:
  PoissonSourceStream(const Name &name, Intensity intensity, Size load_size);

  void init() override;
  EventPtr produce_load(Time time) override;
  void notify_on_send(const LoadSendEvent *event) override;
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PoissonSourceStream &source);
