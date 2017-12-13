
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
                                         Intensity intensity,
                                         Size load_size)
  : SourceStream(world), intensity_(intensity), load_size_(load_size)
{
}

Load PoissonSourceStream::get(Time t)
{
  // auto pf = uniform(world_.get_random_engine());
  // auto dt = -std::log(1.0 - pf) / intensity_ ;
  // auto dt = load_size_/intensity_;

  auto dt = static_cast<Time>(exponential(world_.get_random_engine()));
  auto create_load = [this, t, dt]() -> Load {
    return {world_.get_unique_id(), t + dt, load_size_, -1, {},
            make_observer(this)};
  };
  auto load = create_load();
  debug_print("{} Produced: {}\n", *this, load);

  return load;
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const PoissonSourceStream &source)
{
  f.writer().write("[Source {}]", source.id);
}
