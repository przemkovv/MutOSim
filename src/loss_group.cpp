
#include "loss_group.h"

#include "logger.h"
#include "source_stream/source_stream.h"

LossGroup::LossGroup(
    GroupName name,
    std::unordered_map<TrafficClassId, LostServedStats> &served_by_tc_)
  : name_(std::move(name)), served_by_tc(served_by_tc_)
{
}
bool LossGroup::serve(Load load)
{
  debug_print("{} Load droped. {}\n", *this, load);
  auto &lost = served_by_tc[load.tc_id].lost;
  lost.size += load.size;
  lost.count++;
  return false;
}

//----------------------------------------------------------------------
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const LossGroup &loss_group)
{
  f.writer().write("[LossGroup {}]", loss_group.name_);
}