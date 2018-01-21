
#pragma once
#include "source_stream.h"

#include "topology_parser.h"
#include "traffic_class.h"

#include <memory>

std::unique_ptr<SourceStream> create_stream(Config::SourceType type,
                                            const Config::Source &source,
                                            const TrafficClass &tc);
