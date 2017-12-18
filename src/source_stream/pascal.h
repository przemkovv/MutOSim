
#pragma once

#include "source_stream.h"

#include <map>
#include <random>

class PascalSourceStream : public SourceStream
{
  double intensity_;
  Size load_size_;
  Size sources_number_;
  Size active_sources_ = 0;

  std::map<Uuid, observer_ptr<LoadProduceEvent>> linked_sources_;

  std::exponential_distribution<> exponential{intensity_};

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

  PascalSourceStream(const Name &name,
                     Intensity intensity,
                     Size sources_number,
                     Size load_size);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const PascalSourceStream &source);
