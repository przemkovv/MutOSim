
#pragma once

#include "source_stream.h"

#include <random>
#include <unordered_map>

class PascalSourceStream : public SourceStream
{
  Count sources_number_;
  Count active_sources_ {0};

  std::unordered_map<LoadId, observer_ptr<LoadProduceEvent>> linked_sources_;

  std::exponential_distribution<long double> exponential{ts::get(tc_.source_intensity)};

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

  PascalSourceStream(const SourceName &name,
                    const TrafficClass &tc,
                     Count sources_number);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PascalSourceStream &source);
