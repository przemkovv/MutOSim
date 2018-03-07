
#include "topology_parser.h"
#include "logger.h"
#include "types_parser.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using nlohmann::json;

namespace std
{
template <typename T>
void to_json(json &j, const std::optional<T> &opt)
{
  if (opt) {
    j = *opt;
  } else {
    j = nullptr;
  }
}
template <typename T>
void from_json(const json &j, std::optional<T> &opt)
{
  if (j.is_null()) {
    opt = std::nullopt;
  } else {
    opt = j.get<T>();
  }
}
} // namespace std

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

void to_json(json &j, const CompressionRatio &c);
void from_json(const json &j, CompressionRatio &c);

void to_json(json &j, const TrafficClassSettings &c);
void from_json(const json &j, TrafficClassSettings &c);

void to_json(json &j,
             const std::unordered_map<TrafficClassId, TrafficClassSettings> &tcs);
void from_json(const json &j,
               std::unordered_map<TrafficClassId, TrafficClassSettings> &tcs);

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
       {"intensity_multiplier", g.intensity_multiplier},
       {"overflow_policy", g.overflow_policy},
       {"traffic_classes", g.traffic_classess_settings}};
}
void from_json(const json &j, Group &g)
{
  g.capacity = j.at("capacity");
  g.layer = j.at("layer");
  g.intensity_multiplier = j.value("intensity_multiplier", Intensity{1.0L});
  g.connected = j.value("connected", std::vector<GroupName>{});
  if (j.count("overflow_policy")) {
    g.overflow_policy = j.at("overflow_policy");
  }
  if (j.find("traffic_classes") != j.end()) {
    g.traffic_classess_settings =
        j.at("traffic_classes")
            .get<std::unordered_map<TrafficClassId, TrafficClassSettings>>();
  }
}

void to_json(json &j, const CompressionRatio &c)
{
  j = {{"threshold", c.threshold},
       {"size", c.size},
       {"intensity_factor", c.intensity_factor}};
}
void from_json(const json &j, CompressionRatio &c)
{
  c.threshold = j.at("threshold");
  c.size = j.at("size");
  c.intensity_factor = j.at("intensity_factor");
}

void to_json(json &j, const TrafficClassSettings &c)
{
  if (!c.compression_ratios.empty()) {
    j["compression"] = c.compression_ratios;
  }
  j["block"] = c.block;
}

void from_json(const json &j, TrafficClassSettings &c)
{
  if (j.find("compression") != j.end()) {
    c.compression_ratios = j["compression"].get<std::vector<CompressionRatio>>();
  }
  std::sort(
      begin(c.compression_ratios), end(c.compression_ratios),
      [](const auto &cr1, const auto &cr2) { return cr1.threshold < cr2.threshold; });

  if (j.find("block") != j.end()) {
    c.block = j["block"].get<bool>();
  }
}

void to_json(json &j, const std::unordered_map<TrafficClassId, TrafficClassSettings> &tcs)
{
  for (const auto &[tc_id, tc_s] : tcs) {
    j[std::to_string(get(tc_id))] = tc_s;
  }
}
void from_json(const json &j,
               std::unordered_map<TrafficClassId, TrafficClassSettings> &tcs)
{
  for (const auto &tc_j : j.items()) {
    tcs.emplace(TrafficClassId{std::stoul(tc_j.key())},
                tc_j.value().get<TrafficClassSettings>());
  }
}

void to_json(json &j, const Topology &t)
{
  j["name"] = t.name;
  json tcs_j = {};
  for (const auto &[id, tc] : t.traffic_classes) {
    tcs_j[std::to_string(ts::get(id))] = tc;
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
  const json default_tc = [&]() {
    if (j["traffic_classes"].count("_default") == 1) {
      return j["traffic_classes"].at("_default");
    } else {
      return json{};
    }
  }();
  auto merge = [](const auto &j1, const auto &j2) {
    json result = j1;
    result.update(j2);
    return result;
  };
  for (const auto &tc_j : j["traffic_classes"].items()) {
    if (tc_j.key() == "_default")
      continue;
    TrafficClass tc = merge(default_tc, tc_j.value());
    tc.id = TrafficClassId{stoul(tc_j.key())};
    t.traffic_classes.emplace(tc.id, std::move(tc));
  }
  for (const auto &source_j : j["sources"].items()) {
    Source source = source_j.value();
    source.name = SourceName{source_j.key()};
    t.sources.push_back(source);
  }
  for (const auto &group_j : j["groups"].items()) {
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
