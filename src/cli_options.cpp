
#include "cli_options.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace Model
{
std::istream &
operator>>(std::istream &in, Model::AnalyticModel &model)
{
  std::string token;
  in >> token;
  if (token == "KRFixedCapacity") {
    model = Model::AnalyticModel::KaufmanRobertsFixedCapacity;
  } else if (token == "KRFixedReqSize") {
    model = Model::AnalyticModel::KaufmanRobertsFixedReqSize;
  } else {
    throw boost::program_options::validation_error(
        boost::program_options::validation_error::invalid_option_value,
        "Invalid AnalyticModel");
  }
  return in;
}
std::ostream &
operator<<(std::ostream &out, const Model::AnalyticModel &model)
{
  switch (model) {
  case Model::AnalyticModel::KaufmanRobertsFixedCapacity:
    return out << "KRFixedCapacity";
  case Model::AnalyticModel::KaufmanRobertsFixedReqSize:
    return out << "KRFixedReqSize";
  }
  return out;
}
} // namespace Model
std::ostream &
operator<<(std::ostream &out, const AnalyticModels &models)
{
  for (const auto &model : models) {
    out << model << "; ";
  }
  return out;
}

std::istream &
operator>>(std::istream &in, Mode &mode)
{
  std::string token;
  in >> token;
  if (token == "simulation") {
    mode = Mode::Simulation;
  } else if (token == "analytic") {
    mode = Mode::Analytic;
  } else {
    throw boost::program_options::validation_error(
        boost::program_options::validation_error::invalid_option_value, "Invalid Mode");
  }
  return in;
}
std::ostream &
operator<<(std::ostream &out, const Mode &mode)
{
  switch (mode) {
  case Mode::Simulation:
    return out << "simulation";
  case Mode::Analytic:
    return out << "analytic";
  }
  return out;
}
std::ostream &
operator<<(std::ostream &out, const Modes &modes)
{
  for (const auto &mode : modes) {
    out << mode << "; ";
  }
  return out;
}

boost::program_options::options_description
prepare_options_description()
{
  namespace po = boost::program_options;
  /* clang-format off */
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("scenario-file,f", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
                        "a file with scenario")
    ("append-scenario-files,a", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
                        "a file with a patch scenario that will be merge after the main scenario")
    ("scenarios-dir,I", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
                        "a directories with scenario files")
    ("output-file,o", po::value<std::string>()->default_value(""),
                        "output file with stats")
    ("output-dir,d", po::value<std::string>()->default_value(""),
                        "output directory")
    ("duration,t", po::value<time_type>()->default_value(100'000),
                        "duration of the simulation")
    ("parallel,p", po::value<bool>()->default_value(true),
                        "run simulations in parallel")
    ("start", po::value<intensity_t>()->default_value(0.5L),
                        "starting intensity per group (included)")
    ("stop", po::value<intensity_t>()->default_value(3.0L),
                        "end intensity per group (not included)")
    ("step", po::value<intensity_t>()->default_value(0.5L),
                        "step intensity per group")
    ("count,c", po::value<int>()->default_value(1),
                        "number of repeats of each scenario")
    ("quiet,q", po::value<bool>()->default_value(false),
                        "do not print stats")
    ("mode,m", po::value<Modes>()->multitoken()
                                 ->default_value(Modes{Mode::Simulation})
                                 ->zero_tokens(),
                        "Selects mode in which run simulation:\n"
                        " - analytic\n"
                        " - simulation\n"
                        "Parameter can be repeated")
    ("analytic_model", po::value<AnalyticModels>()->multitoken()
                                                  ->zero_tokens(),
                        "Choose analytic models to consider:\n"
                        " - KRFixedCapacity\n"
                        " - KRFixedReqSize\n"
                        "Parameter can be repeated")
    ("random,r",  po::value<bool>()->default_value(false),
                        "use random seed");
  /* clang-format on */
  return desc;
}
//----------------------------------------------------------------------

CLIOptions
parse_args(const boost::program_options::variables_map &vm)
{
  CLIOptions cli;
  cli.help = vm.count("help") > 0;
  cli.use_random_seed = vm["random"].as<bool>();
  cli.quiet = vm.count("quiet") > 0;
  cli.output_file = vm["output-file"].as<std::string>();
  cli.output_dir = vm["output-dir"].as<std::string>();
  cli.parallel = vm["parallel"].as<bool>();
  cli.duration = Duration{vm["duration"].as<time_type>()};
  cli.A_start = Intensity{vm["start"].as<intensity_t>()};
  cli.A_stop = Intensity{vm["stop"].as<intensity_t>()};
  cli.A_step = Intensity{vm["step"].as<intensity_t>()};
  cli.count = vm["count"].as<int>();
  cli.modes = vm["mode"].as<Modes>();

  cli.analytic_models = [&vm]() -> AnalyticModels {
    if (vm.count("analytic_model") > 0) {
      return vm["analytic_model"].as<AnalyticModels>();
    }
    return {};
  }();

  cli.append_scenario_files = [&vm]() -> std::vector<std::string> {
    if (vm.count("append-scenario-files") > 0) {
      return vm["append-scenario-files"].as<std::vector<std::string>>();
    }
    return {};
  }();
  cli.scenario_files = [&vm]() -> std::vector<std::string> {
    if (vm.count("scenario-file") > 0) {
      return vm["scenario-file"].as<std::vector<std::string>>();
    }
    return {};
  }();
  cli.scenarios_dirs = [&vm]() -> std::vector<std::string> {
    if (vm.count("scenarios-dir") > 0) {
      return vm["scenarios-dir"].as<std::vector<std::string>>();
    }
    return {};
  }();
  return cli;
}
