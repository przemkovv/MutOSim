

import statistics
import itertools
from pprint import pprint
from typing import List, Dict, Tuple
from dataclasses import dataclass, field
from collections import defaultdict
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
from matplotlib.pyplot import Axes
#  from matplotlib.pyplot import Figure
#  import numpy as np
#  import scipy as sp
#  import scipy.stats
from MutOSim.common import describe_dict, get_corresponding_group
from MutOSim.common import YELLOW, BLUE, GREEN, RED
from MutOSim.common import load_data, remove_prefix
from MutOSim.common import print_scenarios, filter_data

Property = str
GroupName = str
ScenarioIndex = int
TcId = int


@dataclass
class Sequence:
    """Represent a single sequence of data together with means."""
    name: str
    x_data: List[float]
    y_data: List[List[float]]
    y_data_mean: List[float] = field(init=False)
    x_range: Tuple[float, float] = field(init=False)

    def __post_init__(self):
        """Compute means."""
        self.y_data_mean = [statistics.mean(subserie)
                            for subserie in self.y_data]
        self.x_range = (min(self.x_data), max(self.x_data))

    def can_use_boxplot(self):
        return len(self.y_data[0]) > 1


@dataclass
class ScenarioSequence:
    """Represent a single sequence of data together with means."""
    name: str
    key: str
    index: int
    sequence: Sequence


@dataclass
class SequenceStyle:
    marker: str = None
    color: str = None
    line: str = None
    markevery: int = 5


def extract_data_sequence(property_name: Property,
                          group_name: GroupName,
                          tc_id: TcId,
                          scenario_data: dict):
    """Parse data from the certain scenario and prepare a sequence."""
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

    tc_size = scenario_data["_scenario"]["traffic_classes"][str(tc_id)]["size"]
    name = f"$t_{tc_id}={tc_size}$"
    assert len(x_data) == len(y_data)
    return Sequence(name, x_data, y_data)


def nested_dict():
    return defaultdict(nested_dict)


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


def plot_sequence(axes: Axes,
                  sequence: Sequence,
                  style: SequenceStyle,
                  enable_boxplots: bool = False,
                  label_suffix: str = None):
    """."""
    if label_suffix is None:
        label_suffix = ""

    if enable_boxplots:
        axes.boxplot(
            sequence.y_data,
            positions=sequence.x_data,
            notch=False,
            widths=0.03,
            bootstrap=10000,
            showfliers=False,
            vert=True,
            patch_artist=False,
            showcaps=False,
            whis=0,
            manage_xticks=False,
        )

    print(sequence.name)
    axes.plot(
        sequence.x_data,
        sequence.y_data_mean,
        label=f"{sequence.name}{label_suffix}",
        marker=style.marker,
        color=style.color,
        linestyle=style.line,
        markevery=style.markevery
    )


class Plots:
    """."""

    sequences: Dict[Property,
                    Dict[ScenarioIndex,
                         Dict[GroupName,
                              Dict[TcId, Sequence]]]] = nested_dict()

    markers = ["+", "x", "2"]
    colors = ["r", "#999900", "b"]
    line_styles = [":", "-", "--"]

    def __init__(self, data_filename: str):
        self.all_scenarios_data = load_data(data_filename)
        self.scenarios_data = self.all_scenarios_data
        self.markerscycle = itertools.cycle(self.markers)
        self.colorscycle = itertools.cycle(self.colors)

    def print_all(self):
        """Print all loaded scenarios."""
        print_scenarios("All scenarios", self.all_scenarios_data)

    def print_selected(self):
        """Print currently selected scenarios."""
        print_scenarios("Selected scenarios", self.scenarios_data)

    def remove_prefix(self, prefix_to_remove: str):
        """Remove prefix from all scenarios names."""
        self.all_scenarios_data = {remove_prefix(k, prefix_to_remove): v
                                   for k, v in self.all_scenarios_data.items()}

    def select_scenarios(self, indices: List[ScenarioIndex]):
        """."""
        self.scenarios_data = filter_data(indices, self.all_scenarios_data)

    def data_sequence_exists(self,
                             scenario: Tuple[ScenarioIndex, Property],
                             group_name: GroupName,
                             tc_id: TcId):
        """Check if the sequence data already has been parsed."""
        sequences = self.sequences
        property_name = scenario[1]
        if property_name not in sequences:
            return False
        scenario_index = scenario[0]
        if scenario_index not in sequences[property_name]:
            return False
        if group_name not in sequences[property_name][scenario_index]:
            return False
        if tc_id not in sequences[property_name][scenario_index][group_name]:
            return False
        return True

    def get_data_sequence(self,
                          scenario: Tuple[ScenarioIndex, Property],
                          group_name: GroupName,
                          tc_id: TcId):
        """."""
        return self.sequences[scenario[1]][scenario[0]][group_name][tc_id]

    def add_data_sequence(self,
                          scenario: Tuple[ScenarioIndex, Property],
                          group_name: GroupName,
                          tc_id: TcId,
                          sequence: Sequence):
        """."""
        scenario_sequence = self.sequences[scenario[1]][scenario[0]]
        scenario_sequence[group_name][tc_id] = sequence

    def prepare_data(self,
                     scenarios: List[Tuple[ScenarioIndex, Property]],
                     group_name: GroupName,
                     traffic_classes: List[TcId]) -> List[ScenarioSequence]:
        """."""
        scenarios_data = self.scenarios_data
        scenarios_results: List[ScenarioSequence] = []
        for tc_id in traffic_classes:
            for scenario in scenarios:
                keys = list(scenarios_data.keys())
                key = keys[scenario[0]]
                index = scenario[1]
                scenario_data = scenarios_data[key]
                name = scenario_data["_scenario"]["name"]
                if not self.data_sequence_exists(scenario,
                                                 group_name,
                                                 tc_id):
                    sequence = extract_data_sequence(index,
                                                     group_name,
                                                     tc_id,
                                                     scenario_data)
                    self.add_data_sequence(scenario,
                                           group_name,
                                           tc_id,
                                           sequence)
                    scenarios_results.append(ScenarioSequence(name,
                                                              key,
                                                              index,
                                                              sequence))
                else:
                    sequence = self.get_data_sequence(scenario,
                                                      group_name,
                                                      tc_id)
                    scenarios_results.append(ScenarioSequence(name,
                                                              key,
                                                              index,
                                                              sequence))
        return scenarios_results

    def get_scenario_key(self, scenario_index: ScenarioIndex) -> str:
        """Return key name of the scenario based on index."""
        return list(self.scenarios_data.keys())[scenario_index]

    def get_scenario_name(self, scenario_index: ScenarioIndex) -> str:
        """Return name of the scenario based on index."""
        scenario_data = self.scenarios_data[self.get_scenario_key(
            scenario_index)]
        return scenario_data["_scenario"]["name"]

    def prepare_title(self,
                      scenario_index: ScenarioIndex,
                      group_name: str) -> str:
        """Prepare title of the plot based on scenario name and group."""
        name = self.get_scenario_name(scenario_index)
        key = self.get_scenario_key(scenario_index)
        return f"{group_name} - {name} ({key})"

    def plot_normal(self,
                    scenarios: List[Tuple[ScenarioIndex, Property]],
                    group_name: GroupName,
                    traffic_classes: List[TcId],
                    title_suffix: str = None):
        """."""
        self.prepare_data(scenarios, group_name, traffic_classes)

        fig = plt.figure(figsize=(6, 4),
                         tight_layout=True)
        title = self.prepare_title(scenarios[0][0], group_name)
        if title_suffix is not None:
            title += title_suffix
        fig.canvas.set_window_title(title)
        axes = fig.add_subplot(1, 1, 1)
        axes.set_title(title)
        set_plot_log_style(axes)
        style = SequenceStyle()
        style.markevery = 1
        colorscycle = itertools.cycle(self.colors)
        linescycle = itertools.cycle(self.line_styles)
        for scenario in scenarios:
            style.color = next(colorscycle)
            style.line = next(linescycle)
            markerscycle = itertools.cycle(self.markers)
            for tc_id in traffic_classes:
                style.marker = next(markerscycle)
                label = self.get_scenario_key(scenario[0]).partition(";")[2]
                if label:
                    label = f" - {label}"
                sequence = self.get_data_sequence(scenario, group_name, tc_id)
                axes.set_xlim(*sequence.x_range)
                plot_sequence(axes,
                              sequence,
                              enable_boxplots=False,
                              style=style,
                              label_suffix=label)
        axes.set_ylabel("P_block")
        axes.set_xlabel("a")
        axes.legend(loc=4, ncol=len(scenarios))
        plt.show(block=False)
