
#pragma once

#include "common.h"
#include "precision.h"

#include <string_view>

namespace ts = type_safe;

namespace Model {

using Intensity = TypesPrecision::Intensity_<highp>;
using IntensityFactor = TypesPrecision::IntensityFactor_<highp>;
using Ratio = TypesPrecision::Ratio_<highp>;
using Capacity = TypesPrecision::Capacity_<highp, use_int_tag>;
using Count = TypesPrecision::Count_<highp>;

using Probability = TypesPrecision::Probability_<highp>;
using MeanIntensity = TypesPrecision::MeanIntensity_<highp>;
using Peakedness = TypesPrecision::Peakedness_<highp>;
using Variance = TypesPrecision::Variance_<highp>;
using MeanRequestNumber = TypesPrecision::MeanRequestNumber_<highp>;

using CapacityF = TypesPrecision::Capacity_<highp, use_float_tag>;
using WeightF = TypesPrecision::Weight_<highp, use_float_tag>;
using SizeF = TypesPrecision::Size_<highp, use_float_tag>;

using SizeRescale = TypesPrecision::SizeRescale_<highp>;

} // namespace Model

namespace Simulation {
using Intensity = TypesPrecision::Intensity_<mediump>;
using IntensityFactor = TypesPrecision::IntensityFactor_<mediump>;
using Ratio = TypesPrecision::Ratio_<mediump>;
using Capacity = TypesPrecision::Capacity_<mediump, use_int_tag>;
using Count = TypesPrecision::Count_<mediump>;
} // namespace Simulation

using Weight = TypesPrecision::Weight_<mediump, use_int_tag>;
using Size = TypesPrecision::Size_<mediump, use_int_tag>;

using Time = TypesPrecision::Time_<mediump>;
using Duration = TypesPrecision::Duration_<mediump>;

struct GroupName : ts::strong_typedef<GroupName, name_t>,
                   ts::strong_typedef_op::equality_comparison<GroupName>,
                   ts::strong_typedef_op::relational_comparison<GroupName>,
                   ts::strong_typedef_op::output_operator<GroupName>
{
  using strong_typedef::strong_typedef;
  GroupName operator+(std::string_view str)
  {
    return GroupName(ts::get(*this) + name_t(str));
  }
};

struct SourceName : ts::strong_typedef<SourceName, name_t>,
                    ts::strong_typedef_op::equality_comparison<SourceName>,
                    ts::strong_typedef_op::relational_comparison<SourceName>,
                    ts::strong_typedef_op::output_operator<SourceName>
{
  using strong_typedef::strong_typedef;
};

struct GroupId : ts::strong_typedef<GroupId, uuid_t>,
                 ts::strong_typedef_op::equality_comparison<GroupId>,
                 ts::strong_typedef_op::output_operator<GroupId>
{
  using strong_typedef::strong_typedef;
};
struct SourceId : ts::strong_typedef<SourceId, uuid_t>,
                  ts::strong_typedef_op::equality_comparison<SourceId>,
                  ts::strong_typedef_op::output_operator<SourceId>
{
  using strong_typedef::strong_typedef;
};
struct LoadId : ts::strong_typedef<LoadId, uuid_t>,
                ts::strong_typedef_op::equality_comparison<LoadId>,
                ts::strong_typedef_op::output_operator<LoadId>
{
  using strong_typedef::strong_typedef;
};

struct TrafficClassId
  : ts::strong_typedef<TrafficClassId, uuid_t>,
    ts::strong_typedef_op::equality_comparison<TrafficClassId>,
    ts::strong_typedef_op::relational_comparison<TrafficClassId>,
    ts::strong_typedef_op::output_operator<TrafficClassId>
{
  using strong_typedef::strong_typedef;
};
