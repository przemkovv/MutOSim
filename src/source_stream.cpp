
#include "source_stream.h"

Request SourceStream::get(Time time)
{
  Request r{to_id(time), 1};
  return r;
}

PoissonSourceStream::PoissonSourceStream(World &world,
                                         double intensity,
                                         TimePeriod time_period)
  : SourceStream(world),
    intensity_(intensity),
    time_period_(time_period),
    d_(static_cast<double>(intensity_) * time_period_)
{
}

double PoissonSourceStream::Pk(const int k, const Time t)
{
  return std::pow(intensity_ * t, k) / Math::factorial(k) *
         exp(-intensity_ * t);
}

Request PoissonSourceStream::get(Time t)
{
  return {to_id(t), d_(world_.get_random_engine())};
}
