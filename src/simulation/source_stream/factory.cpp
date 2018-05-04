
#include "factory.h"

#include "engset.h"
#include "pascal.h"
#include "poisson.h"

namespace Simulation
{
std::unique_ptr<SourceStream>
create_stream(
    Config::SourceType type, const Config::Source &source, const TrafficClass &tc)
{
  switch (type) {
  case Config::SourceType::Poisson:
    return std::make_unique<PoissonSourceStream>(source.name, tc);
  case Config::SourceType::Pascal:
    return std::make_unique<PascalSourceStream>(source.name, tc, source.source_number);
  case Config::SourceType::Engset:
    return std::make_unique<EngsetSourceStream>(source.name, tc, source.source_number);
  }
}
} // namespace Simulation
