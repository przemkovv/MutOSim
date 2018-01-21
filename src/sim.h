
#pragma once

#include "config.h"
#include "types.h"

#include <string>
#include <vector>
#include <boost/program_options.hpp>

struct CLI {
  bool help;
  bool use_random_seed;
  bool quiet;
  std::string output_file;
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
