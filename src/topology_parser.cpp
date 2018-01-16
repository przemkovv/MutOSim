
#include "logger.h"
#include "topology_parser.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using nlohmann::json;

void to_json(json &j, const Weight &s);
void from_json(const json &j, Weight &s);

void to_json(json &j, const Capacity &s);
void from_json(const json &j, Capacity &s);

void to_json(json &j, const Intensity &s);
void from_json(const json &j, Intensity &s);

void to_json(json &j, const TrafficClassId &s);
void from_json(const json &j, TrafficClassId &s);

void to_json(json &j, const Count &s);
void from_json(const json &j, Count &s);

void to_json(json &j, const Size &s);
void from_json(const json &j, Size &s);

void to_json(json &j, const GroupName &gn);
void from_json(const json &j, GroupName &gn);

void to_json(json &j, const SourceName &gn);
void from_json(const json &j, SourceName &gn);

void to_json(json &j, const TrafficClassId &s)
{
  j = ts::get(s);
}
void from_json(const json &j, TrafficClassId &s)
{
  s = TrafficClassId(j.get<ts::underlying_type<TrafficClassId>>());
}

void to_json(json &j, const Count &s)
{
  j = ts::get(s);
}
void from_json(const json &j, Count &s)
{
  s = Count(j.get<ts::underlying_type<Count>>());
}
void to_json(json &j, const Weight &s)
{
  j = ts::get(s);
}
void from_json(const json &j, Weight &s)
{
  s = Weight(j.get<ts::underlying_type<Weight>>());
}
void to_json(json &j, const Capacity &s)
{
  j = ts::get(s);
}
void from_json(const json &j, Capacity &s)
{
  s = Capacity(j.get<ts::underlying_type<Capacity>>());
}
void to_json(json &j, const Intensity &s)
{
  j = ts::get(s);
}
void from_json(const json &j, Intensity &s)
{
  s = Intensity(j.get<ts::underlying_type<Intensity>>());
}

void to_json(json &j, const Size &s)
{
  j = ts::get(s);
}
void from_json(const json &j, Size &s)
{
  s = Size(j.get<ts::underlying_type<Size>>());
}

void to_json(json &j, const SourceName &gn)
{
  j = ts::get(gn);
}
void from_json(const json &j, SourceName &gn)
{
  gn = SourceName(j.get<ts::underlying_type<SourceName>>());
}

void to_json(json &j, const GroupName &gn)
{
  j = ts::get(gn);
}
void from_json(const json &j, GroupName &gn)
{
  gn = GroupName(j.get<ts::underlying_type<GroupName>>());
}

namespace Config
{
void to_json(json &j, const SourceType &st);
void from_json(const json &j, SourceType &st);

void to_json(json &j, const TrafficClass &tc);
void from_json(const json &j, TrafficClass &tc);

void to_json(json &j, const Source &st);
void from_json(const json &j, Source &st);

void to_json(json &j, const Topology &t);
void from_json(const json &j, Topology &t);

void to_json(json &j, const Group &st);
void from_json(const json &j, Group &st);
void to_json(json &j, const SourceType &st)
{
  j = [=]() {
    switch (st) {
    case SourceType::Poisson:
      return "poisson";
    case SourceType::Engset:
      return "engset";
    case SourceType::Pascal:
      return "pascal";
    }
  }();
}

void from_json(const json &j, SourceType &st)
{
  const auto str = j;
  static const std::unordered_map<std::string, SourceType> m = {
      {"poisson", SourceType::Poisson},
      {"engset", SourceType::Engset},
      {"pascal", SourceType::Pascal},
  };

  const auto it = m.find(str);
  st = it->second;
}

void to_json(json &j, const TrafficClass &tc)
{
  j = {{"micro", tc.serve_intensity},
       {"size", tc.size},
       {"weight", tc.weight},
       {"max_path_length", tc.max_path_length}};
}
void from_json(const json &j, TrafficClass &tc)
{
  tc.serve_intensity = Intensity(j.at("micro"));
  tc.size = Size(j.at("size"));
  tc.weight = Weight(j.at("weight"));
  tc.max_path_length = Length(j.value("max_path_length", MaxPathLength));
}

void to_json(json &j, const Source &s)
{
  j = {{"type", s.type}, {"traffic_class", s.tc_id}, {"attached", s.attached}};
  if (s.type == SourceType::Pascal) {
    j["S"] = s.source_number;
  } else if (s.type == SourceType::Engset) {
    j["N"] = s.source_number;
  }
}
void from_json(const json &j, Source &s)
{
  s.type = j.at("type");
  s.tc_id = j.at("traffic_class");
  s.attached = j.at("attached");
  if (s.type == SourceType::Pascal) {
    s.source_number = j.at("S");
  } else if (s.type == SourceType::Engset) {
    s.source_number = j.at("N");
  }
}

void to_json(json &j, const Group &g)
{
  j = {{"capacity", g.capacity},
       {"connected", g.connected},
       {"layer", g.layer},
       {"intensity_multiplier", g.intensity_multiplier}};
}
void from_json(const json &j, Group &g)
{
  g.capacity = j.at("capacity");
  g.layer = j.at("layer");
  g.intensity_multiplier = j.value("intensity_multiplier", Intensity{1.0L});
  g.connected = j.value("connected", std::vector<GroupName>{});
}

void to_json(json &j, const Topology &t)
{
  j["name"] = t.name;
  json tcs_j = {};
  for (const auto &tc : t.traffic_classes) {
    tcs_j[std::to_string(ts::get(tc.id))] = tc;
  }
  j["traffic_classes"] = tcs_j;

  json sources_j;
  for (const auto &source : t.sources) {
    sources_j[ts::get(source.name)] = source;
  }
  j["sources"] = sources_j;

  json groups_j;
  for (const auto &[name, group] : t.groups) {
    groups_j[ts::get(name)] = group;
  }
  j["groups"] = groups_j;
}
void from_json(const json &j, Topology &t)
{
  t.name = j["name"];
  for (const auto &tc_j : json::iterator_wrapper(j["traffic_classes"])) {
    TrafficClass tc = tc_j.value();
    tc.id = TrafficClassId{stoul(tc_j.key())};
    t.traffic_classes.push_back(tc);
  }
  for (const auto &source_j : json::iterator_wrapper(j["sources"])) {
    Source source = source_j.value();
    source.name = SourceName{source_j.key()};
    t.sources.push_back(source);
  }
  for (const auto &group_j : json::iterator_wrapper(j["groups"])) {
    Group group = group_j.value();
    group.name = GroupName{group_j.key()};
    t.groups.emplace(group.name, group);
  }
}

Topology parse_topology_config(std::string_view filename)
{
  std::ifstream file(std::string{filename});
  return json::parse(file);
}

void dump(const Topology &topology)
{
  json j = topology;
  print("[Topology]: {}", j.dump(2));
}

} // namespace Config
