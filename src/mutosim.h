
#pragma once

#include "config.h"
#include "scenario_settings.h"
#include "types.h"

#include <boost/filesystem.hpp>
#include <nlohmann/json_fwd.hpp>
#include <range/v3/algorithm/find.hpp>
#include <string>
#include <vector>

struct CLIOptions;

namespace fs = boost::filesystem;

void print_state(const std::vector<bool> &states);
nlohmann::json
run_scenarios(std::vector<ScenarioSettings> &scenarios, const CLIOptions &cli);

void load_scenarios_from_files(
    std::vector<ScenarioSettings> &scenarios,
    const std::vector<std::string> &scenario_files,
    const CLIOptions &cli);
void
prepare_custom_scenarios(std::vector<ScenarioSettings> &scenarios, const CLIOptions &cli);

std::vector<std::string> find_all_scenario_files(const std::string &path);

void save_json(const nlohmann::json &j, const fs::path &dir, const fs::path &filename);
void print_stats(const std::vector<ScenarioSettings> &scenarios);
