
#include "logger.h"
#include "source_stream.h"

#include <algorithm>
#include <experimental/memory>
#include <iterator>

using std::experimental::make_observer;

Load SourceStream::get(Time time)
{
  return {world_.get_unique_id(), time, 1};
}

PoissonSourceStream::PoissonSourceStream(World &world,
                                         double intensity,
                                         Size load_size)
  : SourceStream(world), intensity_(intensity), load_size_(load_size)
{
}

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
