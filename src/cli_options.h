#pragma once

#include "types.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <iosfwd>
#include <string>
#include <vector>

enum class Mode { Simulation, Analytic };
enum class AnalyticModel { KaufmanRobbertFixedCapacity, KaufmanRobertsFixedReqSize };

using Modes = std::vector<Mode>;
using AnalyticModels = std::vector<AnalyticModel>;


std::istream &operator>>(std::istream &in, Mode &mode);
std::ostream &operator<<(std::ostream &out, const Mode &mode);
std::ostream &operator<<(std::ostream &out, const Modes &modes);
std::istream &operator>>(std::istream &in, AnalyticModel &model);
std::ostream &operator<<(std::ostream &out, const AnalyticModel &model);
std::ostream &operator<<(std::ostream &out, const AnalyticModels &models);

struct CLIOptions {
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
  Modes modes{};
  AnalyticModels analytic_models{};
  int count{};

  std::vector<std::string> append_scenario_files{};
  std::vector<std::string> scenario_files{};
  std::vector<std::string> scenarios_dirs{};
};

CLIOptions parse_args(const boost::program_options::variables_map &vm);
boost::program_options::options_description prepare_options_description();