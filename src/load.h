#pragma once

#include "types.h"
#include <experimental/memory>
#include <fmt/format.h>

struct Group;
class SourceStream;

struct Load {
  Uuid id;
  Time send_time;
  Size size;
  Time end_time = -1;

  std::experimental::observer_ptr<Group> served_by{};
  std::experimental::observer_ptr<SourceStream> produced_by{};
};

bool operator<(const Load &l1, const Load &l2);

class by_send_time
{
public:
  bool operator()(const Load &l1, const Load &l2)
  {
    return l1.send_time < l2.send_time;
  }
};

class by_end_time
{
public:
  bool operator()(const Load &l1, const Load &l2)
  {
    return l1.end_time < l2.end_time;
  }
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const Load &load);
