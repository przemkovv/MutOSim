
#include "source_stream.h"
#include <algorithm>
#include <iterator>

std::vector<Load> SourceStream::get(Time time)
{
  Load r{world_.get_unique_id(), time, 1};
  return {std::move(r)};
}

PoissonSourceStream::PoissonSourceStream(World &world,
                                         double intensity,
                                         Size load_size,
                                         TimePeriod time_period)
  : SourceStream(world),
    intensity_(intensity),
    time_period_(time_period),
    load_size_(load_size),
    d_(static_cast<double>(intensity_) * time_period_)
{
}

double PoissonSourceStream::Pk(const int k, const Time t)
{
  return std::pow(intensity_ * t, k) / Math::factorial(k) *
         exp(-intensity_ * t);
}

std::vector<Load> PoissonSourceStream::get(Time t)
{
  auto loads_count = d_(world_.get_random_engine());
  std::vector<Load> loads;
  loads.reserve(loads_count);
  auto create_load = [this,t]() -> Load {
    return {world_.get_unique_id(), t, load_size_};
  };
  std::generate_n(std::back_inserter(loads), loads_count, create_load);

  return loads;
}
