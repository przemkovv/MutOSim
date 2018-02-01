
#pragma once

#include "config.h"
#include "scenario_settings.h"
#include "types.h"

#include <boost/program_options.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

struct CLI {
  bool help;
  bool use_random_seed;
  bool quiet;
  std::string output_file;
  std::string output_dir;
  bool parallel;
  Duration duration;
  Intensity A_start;
  Intensity A_stop;
  Intensity A_step;
  int count;

  std::vector<std::string> scenario_files;
};

CLI parse_args(const boost::program_options::variables_map &vm);
boost::program_options::options_description prepare_options_description();
void print_state(const std::vector<bool> &states);
nlohmann::json run_scenarios(std::vector<ScenarioSettings> &scenarios, const CLI &cli);
void load_scenarios_from_files(std::vector<ScenarioSettings> &scenarios, const CLI &cli);
void prepare_custom_scenarios(std::vector<ScenarioSettings> &scenarios, const CLI &cli);
