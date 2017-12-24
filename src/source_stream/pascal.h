
#pragma once

#include "source_stream.h"

#include <random>
#include <unordered_map>

class PascalSourceStream : public SourceStream
{
  Intensity intensity_;
  Size load_size_;
  Count sources_number_;
  Count active_sources_ {0};

  std::unordered_map<LoadId, observer_ptr<LoadProduceEvent>> linked_sources_;

  std::exponential_distribution<long double> exponential{ts::get(intensity_)};

  friend void format_arg(fmt::BasicFormatter<char> &f,
                         const char *&format_str,
                         const PascalSourceStream &source);

  std::unique_ptr<LoadProduceEvent> create_produce_load_event(Time time);

  EventPtr produce_load(Time time);

public:
  void init() override;
  void notify_on_send(const LoadSendEvent *event) override;
  void notify_on_serve(const LoadServeEvent *event) override;
  void notify_on_accept(const LoadSendEvent *event) override;
  void notify_on_produce(const LoadProduceEvent *event) override;

  Size get_load_size() override;
  Intensity get_intensity() override;
  PascalSourceStream(const SourceName &name,
                     Intensity intensity,
                     Count sources_number,
                     Size load_size);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PascalSourceStream &source);
