#pragma once

#include "source_stream.h"

#include <random>

class EngsetSourceStream : public SourceStream
{
  double intensity_;
  Size load_size_;
  Size sources_number_;
  Size active_sources_ = 0;

  std::exponential_distribution<> exponential{intensity_};

  friend void format_arg(fmt::BasicFormatter<char> &f,
                         const char *&format_str,
                         const EngsetSourceStream &source);

  std::unique_ptr<LoadProduceEvent> create_produce_load_event(Time time);

  EventPtr produce_load(Time time);

public:
  void init() override;
  void notify_on_serve(const LoadServeEvent *event) override;
  void notify_on_produce(const LoadProduceEvent *event) override;

  EngsetSourceStream(const Name &name,
                     Intensity intensity,
                     Size sources_number,
                     Size load_size);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const EngsetSourceStream &source);
