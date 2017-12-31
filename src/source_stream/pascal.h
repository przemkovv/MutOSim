
#pragma once

#include "source_stream.h"

#include <random>
#include <unordered_map>

class PascalSourceStream : public SourceStream
{
  Count sources_number_;
  Count active_sources_{0};

  Count linked_sources_count_{0};
  std::unordered_map<LoadId, observer_ptr<Event>> linked_sources_;
  std::unordered_map<LoadId, observer_ptr<Event>> closing_linked_sources_;

  std::exponential_distribution<long double> exponential{
      ts::get(tc_.source_intensity)};

  friend void format_arg(fmt::BasicFormatter<char> &f,
                         const char *&format_str,
                         const PascalSourceStream &source);

  std::unique_ptr<ProduceServiceRequestEvent>
  create_produce_service_request(Time time);

  EventPtr produce_load(Time time);

public:
  void init() override;
  void notify_on_service_start(const LoadServiceRequestEvent *event) override;
  void notify_on_service_end(const LoadServiceEndEvent *event) override;
  void notify_on_service_drop(const LoadServiceRequestEvent *event) override;
  void notify_on_service_accept(const LoadServiceRequestEvent *event) override;
  void notify_on_produce(const ProduceServiceRequestEvent *event) override;

  PascalSourceStream(const SourceName &name,
                     const TrafficClass &tc,
                     Count sources_number);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PascalSourceStream &source);
