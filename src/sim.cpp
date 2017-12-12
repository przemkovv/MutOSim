
#include "config.h"
#include "events.h"
#include "group.h"
#include "source_stream.h"
#include "types.h"
#include "world.h"

#include <fmt/format.h>
#include <iostream>
#include <map>
#include <optional>
#include <random>

// template <>
// PoissonSourceStream<std::mt19937_64>;

uint64_t seed();

uint64_t seed()
{
  if constexpr (Config::constant_seed) {
    return 0;
  } else {
    std::random_device rd;
    return rd();
  }
}

int main()
{
  World world{seed(), Time(0), TimePeriod(1000000), TimePeriod(0.1)};
  PoissonSourceStream source_stream(world, 1, Size(1), world.tick_length);

  // int64_t events_number = 0;

  std::map<uint64_t, uint64_t> events_counts;
  // Size served_load = 0;
  // Size served_load_size = 0;
  // Size blocked_load = 0;
  // Size blocked_load_size = 0;

  auto group = std::make_unique<Group>(world);
  world.add_group(std::move(group));

  world.loads_send_.emplace(source_stream.get(world.time));
  while (world.advance() <= world.length) {
    // for (auto &load : loads) {
    // if (world.serve_load(load)) {
    // served_load++;
    // served_load_size += load.size;
    // } else {
    // blocked_load++;
    // blocked_load_size += load.size;
    // }
  }

  world.print_stats();

  // const auto count = 100000;
  // double sum = 0;
  // std::uniform_real_distribution<> dis{0.0, 1.0};
  // for (int i = 0; i < count; ++i) {
  // auto pf = dis(world.get_random_engine());
  // auto t_serv = -std::log(1.0 - pf);
  // sum += t_serv;
  // }
  // fmt::print("{}\n", sum/count);

  // fmt::print("{}, {}",
  // blocked_load / (blocked_load + served_load),
  // blocked_load_size / (blocked_load_size + served_load_size));

  // fmt::print("\n{}/{}= {}\n",
  // events_number,
  // world.length,
  // static_cast<double>(events_number) / world.length);
  // fmt::print("\n{}/{}= {}\n",
  // events_number,
  // world.length,
  // static_cast<double>(events_number) / world.length);

  // for (auto p : events_counts) {
  // std::cout << p.first << ' '
  // << std::string(static_cast<uint64_t>(
  // std::log10(static_cast<double>(p.second))),
  // '*')
  // << '\n';
  // }

  return 0;
}
