
#include "source_stream.h"
#include "logger.h"
#include <algorithm>
#include <iterator>

#include <experimental/memory>
using std::experimental::make_observer;

Load SourceStream::get(Time time)
{
  Load r{world_.get_unique_id(), time, 1};
  return r;
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

/*
std::vector<Load> PoissonSourceStream::get(Time t)
{
  auto loads_count = d_(world_.get_random_engine());
  std::vector<Load> loads;
  loads.reserve(loads_count);
  auto create_load = [this, t]() -> Load {
    return {world_.get_unique_id(), t + dt, load_size_, -1, {},
            make_observer(this)};
  };
  std::generate_n(std::back_inserter(loads), loads_count, create_load);

  return loads;
}
*/
Load PoissonSourceStream::get(Time t)
{
  auto pf = uniform(world_.get_random_engine());
  auto dt = -std::log(1.0 - pf) / intensity_;
  // auto dt = load_size_/intensity_;

  auto create_load = [this, t, dt]() -> Load {
    return {world_.get_unique_id(), t + dt, load_size_, -1, {},
            make_observer(this)};
  };
  auto load = create_load();
  debug_print("[SourceStream] Produced: {}\n", load);

  return load;
}
