#!/usr/bin/env python

"""
Simulator of network with Mutual Overflows.

Usage:
    plot.py <DATA_FILE> [-p PROPERTY]...
            [--linear]
            [--y_max=Y_MAX] [--y_min=Y_MIN]
            [--x_max=X_MAX] [--x_min=X_MIN]
            [-x X] [-y Y]
            [--save] [--output-dir=DIR]
            [--format=FORMAT]
            [--quiet]
            [--bp] [-i INDICES | --indices=INDICES]
            [-r] [--relatives]
            [--relatives-diffs]
            [--relative-sums]
            [--relative-divs]
            [--normal]
            [--normal_single]
            [-n NAME]
            [--width=WIDTH] [--height=HEIGHT]
            [--pairs=PAIRS]
            [--groups=GROUPS]
            [--tc=TCs]
            [--title-suffix=TITLE_SUFFIX]
            [--no-pair-suffix]
            [--print-all]
    plot.py -h | --help

Arguments:
    DATA_FILE   path to the file with data to plot

Options:
    -h --help                       show this help message and exit
    -p PROPERTY                     property from data file to plot
                                    [default: P_block]
    --linear                        linear plot (default is log)
    --x_max=X_MAX                   right limit on x axis [default: 3.0]
    --x_min=X_MIN                   left limit on x axis [default: 0.0]
    --y_max=Y_MAX                   top limit on y axis [default: 5]
    --y_min=Y_MIN                   bottom limit on y axis [default: 1e-6]
    -x X                            number of plots horizontally [default: 3]
    -y Y                            number of plots vertically [default: 3]
    -s, --save                      save to file
    --format FORMAT                 output format [default: pdf]
    -q, --quiet                     don't show plot window
    -d DIR, --output-dir=DIR        directory where the files are saved
                                    [default: data/results/plots/]
    --bp                            enable box plots
    -i INDICES, --indices=INDICES   indices of scenarios to plot [default: -1]
    -r, --relatives                 plot relations (ratio)
    --relatives-diffs               plot relations (difference)
    --normal                        normal plots
    --normal_single                 single normal plot
    --relative-sums                 plots of relatives sums
    --relative-divs                 plots of relatives divisions
    -n NAME, --name=NAME            suffix added to filename
    --width WIDTH                   width of generated image [default: 32]
    --height HEIGHT                 height of generated image [default: 18]
    --pairs PAIRS                   list of scenario pairs for relative plots
    --no-pair-suffix                skip automatic suffix for pairs
    --print-all                     print all scenarios with ids
    -g GROUPS, --groups=GROUPS      groups that have to be plotted
    --title-suffix=TITLE_SUFFIX     title suffix [default: ]
    --tc=TCs                        filter TCs [default: None]

"""
import os
import json
import math
import statistics
import itertools
import ast
from pprint import pprint
from typing import Optional, List, Iterable, Dict, Tuple
from dataclasses import dataclass, field
from collections import defaultdict
from enum import Enum
from docopt import docopt
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
from matplotlib.pyplot import Axes
from matplotlib.pyplot import Figure
import numpy as np
import scipy as sp
import scipy.stats
import ubjson
import cbor2
import colorama
from colorama import Fore, Style


STAT_NAME_TO_LABEL = {
    "served_u": "Carried traffic",
    "served": "Carried requests",
    "P_block": "Blocking probability",
    "P_block_recursive": "Blocking probability",
}

YELLOW = f"{Fore.YELLOW}{Style.BRIGHT}"
BLUE = f"{Fore.BLUE}{Style.BRIGHT}"
GREEN = f"{Fore.GREEN}"
RED = f"{Fore.RED}{Style.BRIGHT}"


def flip(items, ncol: int):
    """Swap columns with rows in respect to ncol columns."""
    return itertools.chain(*[items[i::ncol] for i in range(ncol)])


def cols_number(items):
    """Determine number of columns used by the items.

    Determine number of columns while trying assign equal number of
    items in rows.
    """
    items_number = len(items)
    max_per_column = 5
    if items_number <= max_per_column:
        return items_number
    if items_number <= 2 * max_per_column:
        return math.ceil(items_number / 2)
    return max_per_column


def get_traffic_classes_sizes(scenario):
    """Extract from the scenario all sizes of requests."""
    tc_sizes = {}
    for tc_id, tc_data in scenario["traffic_classes"].items():
        if tc_id[0] == "_":
            continue
        tc_sizes[int(tc_id)] = tc_data["size"]
    return (tc_sizes, scenario)


def is_group_in_layer(group_name, layer_name):
    """Check if a certain group has been included in a given layer.

    Layer name is created by analytic model.
    """
    if layer_name[0] != "L":
        return False
    return group_name in layer_name_to_groups(layer_name)


def is_layer_name(layer_name):
    """Check if the group has been treated as a whole layer."""
    return layer_name[0] == "L"


def layer_name_to_groups(layer_name: str) -> Iterable[str]:
    """Convert layer name to a list of collection of group names."""
    return filter(None, layer_name.split(":", 1)[1].split(";"))


def is_in_groups(group_name: str, groups: List[str]):
    """Check if the group name is in the set of groups.

    If the group name is a layer name, check if any layer subgroups
    is in groups set.
    """
    if is_layer_name(group_name):
        return any(is_group_in_layer(group, group_name)
                   for group in groups)
    return group_name in groups


def get_valid_group(group_name: str, groups) -> Optional[str]:
    """Find and return a valid group.

    Find and return a valid group that is an intersection of
    group_name and groups. group_name can be a layer name.
    """
    if is_layer_name(group_name):
        layer_name = group_name
        return next(
            (group
             for group in layer_name_to_groups(layer_name)
             if group in groups),
            None,
        )
    if group_name in groups:
        return group_name
    return None


def get_corresponding_group(needle_group, groups):
    """Find an group among all groups or layers."""
    for group in groups:
        if is_layer_name(group):
            layer_name = group
            layer_groups = layer_name_to_groups(layer_name)
            return next(
                (layer_name
                 for layer_group in layer_groups
                 if needle_group == layer_group
                 ),
                None,
            )
        if needle_group == group:
            return group
    return None


def append_tc_stat_for_groups_by_size(tc_data_y,
                                      scenario_result,
                                      stat_name,
                                      tc_sizes):
    """."""
    for group_name, tcs_stats in scenario_result.items():
        if group_name.startswith("_"):
            continue
        group_y = tc_data_y.setdefault(group_name, {})
        new_data = {}
        for tc_id, tc_stats in tcs_stats.items():
            size = tc_sizes[int(tc_id)]
            new_data.setdefault(size, 0)
            if tc_stats[stat_name]:
                if tc_stats[stat_name] != [None]:
                    new_data[size] += statistics.mean(
                        tc_stats[stat_name])
                else:
                    new_data[size] += 0

        for tc_size, data in new_data.items():
            tc_series = group_y.setdefault(tc_size, [])
            tc_series.append(data)


def append_tc_stat_for_groups(tc_data_y,
                              scenario_result,
                              stat_name,
                              tcs_served_by_groups):
    """."""
    for group_name, tcs_stats in scenario_result.items():
        if group_name.startswith("_"):
            continue
        group_y = tc_data_y.setdefault(group_name, {})
        for tc_id in tcs_served_by_groups[group_name]:
            tc_series = group_y.setdefault(int(tc_id), [])
            if tcs_stats is not None and tc_id in tcs_stats:
                tc_stats = tcs_stats[tc_id]
                tc_serie = tc_stats[stat_name]
                tc_series.append(tc_serie)
            else:
                tc_series.append([0.0])


def set_plot_style(axes: Axes):
    """Set plot style."""
    axes.grid(axis="both", which="major", linestyle="-")
    axes.grid(axis="x", which="minor", linestyle="--")
    minor_locator = AutoMinorLocator()
    axes.xaxis.set_minor_locator(minor_locator)


def set_plot_linear_style(axes: Axes, bottom=0, top=15e5):
    """Set linear plot style."""
    set_plot_style(axes)
    axes.set_yscale("linear")
    axes.set_ylim(bottom=bottom, top=top, auto=True, emit=True)


def set_plot_log_style(axes: Axes, bottom=1e-6, top=5):
    """Set logarythmic plot style."""
    set_plot_style(axes)
    axes.set_yscale("log")
    axes.set_ylim(bottom=bottom, top=top, auto=True, emit=True)


def get_tcs_served_by_groups(scenario_results, filtered_tcs):
    """."""
    tcs_served_by_groups = {}
    for _, result in scenario_results.items():
        for group_name, tcs_stats in result.items():
            if group_name.startswith("_") or tcs_stats is None:
                continue
            tcs_served_by_group = tcs_served_by_groups.setdefault(
                group_name, [])
            for tc_id, _ in tcs_stats.items():
                if tc_id not in tcs_served_by_group:
                    if filtered_tcs is None or int(tc_id) in filtered_tcs:
                        tcs_served_by_group.append(tc_id)
    for _, tcs in tcs_served_by_groups.items():
        tcs.sort()
    return tcs_served_by_groups


def get_used_traffic_classes(scenario_results, filtered_tcs):
    """Return list of traffic classes served by any group."""
    tcs_served_by_groups = {}
    for key, result in scenario_results.items():
        if key.startswith("_"):
            continue
        for group_name, tcs_stats in result.items():
            if group_name.startswith("_") or tcs_stats is None:
                continue
            tcs_served_by_group = tcs_served_by_groups.setdefault(
                group_name, [])
            for tc_id, _ in tcs_stats.items():
                if tc_id not in tcs_served_by_group:
                    if filtered_tcs is None or int(tc_id) in filtered_tcs:
                        tcs_served_by_group.append(tc_id)
    for _, tcs in tcs_served_by_groups.items():
        tcs.sort()
    return tcs_served_by_groups


def describe_dict(dictionary: dict, prefix=()):
    """Return an hierarchical structure of the dictionary."""
    for key, value in dictionary.items():
        path = prefix + (key,)
        yield path
        if isinstance(value, dict):
            yield from describe_dict(value, path)


def compare_dicts_structure(dict1, dict2):
    """Compare if two dictionaries structures matches themselves."""
    return sorted(describe_dict(dict1)) == sorted(describe_dict(dict2))


def confidence_interval(data, confidence=0.95):
    """Return data_array values interval for data_array given confidence."""
    data_array = 1.0 * np.array(data)
    length = len(data_array)
    std_err = scipy.stats.sem(data_array)
    h = std_err * sp.stats.t._ppf((1 + confidence) / 2.0, length - 1)
    return h


def clean(text: str):
    """Remove prefixes from the name."""
    return (
        text.replace("data/journal/", "")
        .replace("data/journal2/", "")
        .replace("data/scenarios/simulator_publication/", "")
        .replace("data/scenarios/analytical/layer_types/", "")
        .replace(".json", "")
    )


def load_data(filename: str):
    """Load data from JSON file."""
    ext = os.path.splitext(filename)[1]
    with open(filename, "rb") as data_file:
        if ext == ".json":
            data = json.load(data_file)
        elif ext == ".cbor":
            data = cbor2.load(data_file)
        elif ext == ".ubjson":
            data = ubjson.load(data_file)
        return data


def load_data2(filename: str) -> ([str], Dict[str, dict]):
    """Load data from JSON file."""
    ext = os.path.splitext(filename)[1]
    with open(filename, "rb") as data_file:
        if ext == ".json":
            data = json.load(data_file)
        elif ext == ".cbor":
            data = cbor2.load(data_file)
        elif ext == ".ubjson":
            data = ubjson.load(data_file)
        return dict(sorted(data.items()))


def remove_prefix(text: str, prefix: str) -> str:
    """."""
    if text.startswith(prefix):
        return text[len(prefix):]
    return text


def filter_data(indices_str: str, strip_prefix: str, data):
    """Leave only results from request list of indices."""
    if indices_str == "-1":
        return data

    indices = map(int, indices_str.split(","))
    return {remove_prefix(k, strip_prefix): data[k]
            for k in [sorted(data.keys())[i]
                      for i in indices]
            }


def filter_data2(indices: List[int], data: dict):
    """Leave only results from request list of indices."""
    if not indices:
        return data

    keys: List[str] = list(data.keys())
    return {k: data[k]
            for k in [keys[i] for i in indices]
            }


def print_scenarios(title: str, data):
    """Print an enumerate list of scenario names."""
    print(f"{YELLOW}{title}:")
    for index, scenario_name in enumerate(data.keys()):
        print(f"  {index}: {scenario_name}")


class PlotType(Enum):
    """Define plot types."""

    RelativeDivs = "_relative_divs"
    RelativeSums = "_relative_sums"
    Relatives = "_relatives"
    Normal = "_normal"
    NormalSingle = "_normal_single"


@dataclass
class ScenarioResults:
    """."""

    index: int
    name: str
    filename: str
    tc_data_x: List[float]
    tc_data_y: dict
    tc_data_y_by_size: dict
    tc_sizes: dict


class Main:
    """."""

    markers = ["+", "x", "2"]
    colors = ["r", "g", "b"]
    line_styles = [":", "-"]

    def __init__(self, args: dict):
        """Init plotting class."""
        self.args = args
        data_filename = args["<DATA_FILE>"]
        self.properties = args["-p"]
        self.x_range = (float(args["--x_min"]), float(args["--x_max"]))
        self.y_range = (float(args["--y_min"]), float(args["--x_max"]))
        self.logarithmic_plot = not args["--linear"]
        self.aggregate_over_tc_size = args["--linear"]
        self.enable_boxplots = args["--bp"]
        self.title_suffix = args["--title-suffix"]

        self.strip_prefix = "data/scenarios/analytical/layer_types/"

        self.groups = ast.literal_eval(
            args["--groups"]) if args["--groups"] else None

        self.tc_filter = ast.literal_eval(
            args["--tc"]) if args["--tc"] else []

        self.plot_grid = (int(args["-x"]), int(args["-y"]))

        self.plot_size = (float(args["--width"]), float(args["--height"]))

        basename = clean(os.path.splitext(os.path.basename(data_filename))[0])
        self.title = f"{basename}_{self.properties[0]}"
        if args["--relative-divs"]:
            self.plot_type = PlotType.RelativeDivs
        elif args["--relative-sums"]:
            self.plot_type = PlotType.RelativeSums
        elif args["--relatives"]:
            self.plot_type = PlotType.Relatives
        elif args["--normal"]:
            self.plot_type = PlotType.Normal
        elif args["--normal_single"]:
            self.plot_type = PlotType.NormalSingle

        self.title += self.plot_type.value

        if args["--name"]:
            self.title += args["--name"]

        data = load_data(data_filename)
        if self.args["--print-all"]:
            print_scenarios("All scenarios", data)

        self.source_scenarios_data = filter_data(args["--indices"],
                                                 self.strip_prefix,
                                                 data)
        if len(self.properties) == 1:
            self.properties = self.properties * len(self.source_scenarios_data)

        print_scenarios("Filtered scenarios", self.source_scenarios_data)

        self.scenarios_results = self.prepare_data(self.source_scenarios_data)

    def set_style(self, axes: Axes):
        """Set style of the plot."""
        if self.logarithmic_plot:
            return set_plot_log_style(axes, *self.y_range)
        else:
            return set_plot_linear_style(axes, *self.y_range)

    def prepare_data(self, scenarios_data: dict) -> dict:
        """Parse and prepare data of selected scenarios."""
        scenarios_items = scenarios_data.items()
        scenarios_results: dict = {}
        for index, (filename, scenario_data) in enumerate(scenarios_items):
            property_name = self.properties[index]
            print(f"{GREEN}Parsing '{filename}' scenario"
                  f" (index: {index}, property: '{property_name}').")
            scenarios_results[filename] = self.prepare_scenario_data(
                scenario_data,
                property_name,
                index,
                filename)
        return scenarios_results

    def prepare_scenario_data(self,
                              source_scenario_data: dict,
                              stat_name: str,
                              index: int,
                              filename: str) -> ScenarioResults:
        """Parse and prepare data of a certain scenario."""
        scenario_description = source_scenario_data["_scenario"]
        tc_sizes, _ = get_traffic_classes_sizes(scenario_description)

        tc_data_x: List[float] = []
        tc_data_y: dict = {}
        tc_data_y_by_size: dict = {}

        used_tcs = get_used_traffic_classes(source_scenario_data,
                                            self.tc_filter)

        for key, result in source_scenario_data.items():
            if key.startswith("_"):
                continue
            tc_data_x.append(float(result["_A"]))
            append_tc_stat_for_groups(tc_data_y,
                                      result,
                                      stat_name,
                                      used_tcs)
            if self.aggregate_over_tc_size:
                append_tc_stat_for_groups_by_size(tc_data_y_by_size,
                                                  result,
                                                  stat_name,
                                                  tc_sizes)
        return ScenarioResults(index, scenario_description["name"],
                               filename,
                               tc_data_x=tc_data_x,
                               tc_data_y=tc_data_y,
                               tc_data_y_by_size=tc_data_y_by_size,
                               tc_sizes=tc_sizes)
        #  return scenario_data

    def plot_tc_series(self,
                       axes: Axes,
                       data_x,
                       group_data_y,
                       tc_sizes,
                       label_suffix: str = None,
                       markers: List[str] = None,
                       line_style: str = "-",
                       markevery: int = 1):
        """."""
        if markers is None:
            markers = self.markers
        if label_suffix is None:
            label_suffix = ""

        markerscycle = itertools.cycle(markers)
        colorscycle = itertools.cycle(self.colors)
        for tc_id, data_y in group_data_y.items():
            if self.enable_boxplots:
                axes.boxplot(
                    data_y,
                    positions=data_x,
                    notch=False,
                    widths=0.05,
                    bootstrap=10000,
                    showfliers=False,
                    vert=True,
                    patch_artist=False,
                    showcaps=False,
                    whis=0,
                    manage_xticks=False,
                )

            series_means = [statistics.mean(serie) for serie in data_y]
            axes.plot(
                data_x,
                series_means,
                label=f"$t_{tc_id}={tc_sizes[tc_id]}${label_suffix}",
                marker=next(markerscycle),
                color=next(colorscycle),
                linestyle=line_style,
                markevery=markevery
            )

    def plot_single_normal(self,
                           axes: Axes,
                           group_name: str,
                           scenario_results: ScenarioResults,
                           line_style: str = None,
                           xlabel: bool = True):
        """Plot Normal type."""

        label_suffix = scenario_results.filename.partition(";")[2]
        if label_suffix:
            label_suffix = f" - {label_suffix}"
        if line_style is None:
            line_style = self.line_styles[scenario_results.index]

        group_data_y = scenario_results.tc_data_y[group_name]

        axes.set_xlim(*self.x_range)
        self.plot_tc_series(
            axes,
            scenario_results.tc_data_x,
            group_data_y,
            scenario_results.tc_sizes,
            label_suffix=label_suffix,
            line_style=line_style,
            markevery=5)

        if self.title_suffix:
            axes.set_title(f"{group_name} {self.title_suffix}")
        else:
            title = (f'{group_name} {scenario_results.name}\n'
                     f'({scenario_results.index}: '
                     f'{scenario_results.filename})')
            axes.set_title(axes.get_title() + "\n" + title)
        self.set_style(axes)
        axes.set_ylabel(STAT_NAME_TO_LABEL.get(self.properties[0],
                                               self.properties[0]))
        if xlabel:
            axes.set_xlabel("a")
        axes.legend(loc=4, ncol=3)

    def plot_normal(self,
                    fig: Figure,
                    plot_id: int,
                    scenario_results: ScenarioResults):
        """Plot Normal type."""
        for group_name in scenario_results.tc_data_y.keys():
            if not get_valid_group(group_name, self.groups):
                continue

            axes = fig.add_subplot(*self.plot_grid, plot_id)
            self.plot_single_normal(axes,
                                    group_name,
                                    scenario_results,
                                    line_style=':',
                                    xlabel=plot_id % self.plot_grid[0] == 0)

            plot_id += 1
        return plot_id

    def run(self):
        """."""

    def plot(self):
        """."""
        fig = plt.figure(figsize=self.plot_size,
                         tight_layout=True)
        fig.canvas.set_window_title(self.title)

        if self.plot_type == PlotType.NormalSingle:
            axes = fig.add_subplot(1, 1, 1)
            for scenario_results in self.scenarios_results.values():
                for group_name in scenario_results.tc_data_y.keys():
                    if not get_valid_group(group_name, self.groups):
                        continue
                    self.plot_single_normal(axes,
                                            group_name,
                                            scenario_results)

        if self.plot_type == PlotType.Normal:
            plot_id = 1
            for scenario_results in self.scenarios_results.values():
                plot_id = self.plot_normal(fig,
                                           plot_id,
                                           scenario_results)

        output_file = self.create_filename(self.title)
        fig.set_size_inches(self.plot_size)
        print(f"{BLUE}Saving {output_file}")
        fig.savefig(output_file, transparent=True)

    def create_filename(self, name: str):
        """."""
        output_dir = self.args["--output-dir"]
        if not os.path.isdir(output_dir):
            os.makedirs(output_dir)
        output_file = name + "." + self.args["--format"]
        output_file = os.path.join(output_dir, output_file)
        return output_file


def main2():
    """."""
    args = docopt(__doc__, version="0.1")
    scenario_plot = Main(args)
    scenario_plot.run()
    scenario_plot.plot()


Property = str
GroupName = str
ScenarioIndex = int
TcId = int


@dataclass
class Serie:
    name: str
    x_data: List[float]
    y_data: List[List[float]]
    y_data_mean: List[float] = field(init=False)

    def __post_init__(self):
        self.y_data_mean = [statistics.mean(subserie)
                            for subserie in self.y_data]


def extract_serie_data(property_name: Property,
                       group_name: GroupName,
                       tc_id: TcId,
                       scenario_data: dict):
    """."""
    x_data = []
    y_data = []
    for intensity, intensity_data in scenario_data.items():
        if intensity.startswith("_"):
            continue
        groups = list(intensity_data.keys())
        corresponding_group = get_corresponding_group(group_name, groups)
        if not corresponding_group:
            print(f"{RED} No group {group_name}.")
            pprint(groups)
            continue
        group_data = intensity_data[corresponding_group]
        if str(tc_id) not in group_data:
            print(f"{RED} No tc id {tc_id}.")
            continue
        properties_data = group_data[str(tc_id)]
        x_data.append(float(intensity))
        y_data.append(properties_data[property_name])

    assert len(x_data) == len(y_data)
    return Serie("", x_data, y_data)


def nested_dict():
    return defaultdict(nested_dict)


class MutOSimPlots:
    """."""

    series: Dict[Property,
                 Dict[ScenarioIndex,
                      Dict[GroupName,
                           Dict[TcId, Serie]]]] = nested_dict()

    def __init__(self, data_filename: str):
        self.all_scenarios_data = load_data2(data_filename)
        self.scenarios_data = self.all_scenarios_data

    def print_all(self):
        """."""
        print_scenarios("All scenarios", self.all_scenarios_data)

    def print_selected(self):
        """."""
        print_scenarios("Selected scenarios", self.scenarios_data)

    def remove_prefix(self, prefix_to_remove: str):
        """."""
        self.all_scenarios_data = {remove_prefix(k, prefix_to_remove): v
                                   for k, v in self.all_scenarios_data.items()}

    def select_scenarios(self, indices: List[ScenarioIndex]):
        """."""
        self.scenarios_data = filter_data2(indices, self.all_scenarios_data)

    def serie_exists(self,
                     property_name: Property,
                     group_name: GroupName,
                     scenario_index: ScenarioIndex,
                     tc_id: TcId):
        """."""
        series = self.series
        if property_name not in series:
            return False
        if group_name not in series[property_name]:
            return False
        if scenario_index not in series[property_name][group_name]:
            return False
        if tc_id not in series[property_name][group_name][scenario_index]:
            return False
        return True

    def get_serie(self,
                  property_name: Property,
                  group_name: GroupName,
                  scenario_index: ScenarioIndex,
                  tc_id: TcId):
        """."""
        return self.series[property_name][group_name][scenario_index][tc_id]

    def add_serie(self,
                  property_name: Property,
                  group_name: GroupName,
                  scenario_index: ScenarioIndex,
                  tc_id: TcId,
                  serie: Serie):
        """."""
        self.series[property_name][group_name][scenario_index][tc_id] = serie

    def prepare_data(self,
                     scenarios: List[Tuple[ScenarioIndex, Property]],
                     group_name: GroupName,
                     traffic_classes: List[TcId]):
        """."""
        scenarios_data = self.scenarios_data
        for tc_id in traffic_classes:
            keys = list(scenarios_data.keys())
            for scenario_index, property_name in scenarios:
                scenario_data = scenarios_data[keys[scenario_index]]
                if not self.serie_exists(property_name,
                                         group_name,
                                         scenario_index,
                                         tc_id):
                    serie = extract_serie_data(property_name,
                                               group_name,
                                               tc_id,
                                               scenario_data)
                    self.add_serie(property_name,
                                   group_name,
                                   scenario_index,
                                   tc_id,
                                   serie)
                else:
                    serie = self.get_serie(property_name,
                                           group_name,
                                           scenario_index,
                                           tc_id)

    def plot_normal(self,
                    property_name: Property,
                    group_name: GroupName,
                    traffic_classes: List[TcId]):
        """."""
        self.prepare_data(property_name, group_name, traffic_classes)
        pprint(list(describe_dict(self.series)))


def main3():
    """."""

    output_dir = "data/results/plots/layer_types"
    data_filename = "data/results/layer_types/eq_m.json"
    prefix_to_remove = "data/scenarios/analytical/layer_types/"
    settings = dict(
        x_range=(0.3, 2.4),
        plot_size=(10, 5)
    )
    traffic_classes = [3, 4, 5]

    scenario_plots = MutOSimPlots(data_filename)
    scenario_plots.remove_prefix(prefix_to_remove)

    scenario_plots.select_scenarios(None)
    #  scenario_plots.select_scenarios([6, 8])
    scenario_plots.print_selected()
    scenario_plots.plot_normal([(6, "P_block_recursive"), (8, "P_block")],
                               "G0",
                               traffic_classes)
    #  scenario_plots.plot_normal("P_block_recursive", "G2", traffic_classes)


if __name__ == "__main__":
    colorama.init(autoreset=True)
    main3()
