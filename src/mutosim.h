
#pragma once

#include "config.h"
#include "scenario_settings.h"
#include "types.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace fs = boost::filesystem;

struct CLI {
  bool help = false;
  bool use_random_seed = false;
  bool quiet = false;
  std::string output_file{};
  std::string output_dir{};
  bool parallel = false;
  Duration duration{};
  Intensity A_start{};
  Intensity A_stop{};
  Intensity A_step{};
  bool analytical = false;
  int count{};

  std::vector<std::string> append_scenario_files{};
  std::vector<std::string> scenario_files{};
  std::vector<std::string> scenarios_dirs{};
};

CLI parse_args(const boost::program_options::variables_map &vm);
boost::program_options::options_description prepare_options_description();
void print_state(const std::vector<bool> &states);
nlohmann::json run_scenarios(std::vector<ScenarioSettings> &scenarios, const CLI &cli);

void load_scenarios_from_files(
    std::vector<ScenarioSettings> &scenarios,
    const std::vector<std::string> &scenario_files,
    const CLI &cli);
void prepare_custom_scenarios(std::vector<ScenarioSettings> &scenarios, const CLI &cli);

std::vector<std::string> find_all_scenario_files(const std::string &path);

void save_json(const nlohmann::json &j, const fs::path &dir, const fs::path &filename);
void print_stats(const std::vector<ScenarioSettings> &scenarios);
