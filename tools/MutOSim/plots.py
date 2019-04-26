

import statistics
import itertools
from pprint import pprint
from typing import Optional, List, Iterable, Dict, Tuple
from dataclasses import dataclass, field
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
from matplotlib.pyplot import Axes
from matplotlib.pyplot import Figure
import numpy as np
import scipy as sp
import scipy.stats
from collections import defaultdict

from MutOSim.common import *


Property = str
GroupName = str
ScenarioIndex = int
TcId = int


@dataclass
class Sequence:
    name: str
    x_data: List[float]
    y_data: List[List[float]]
    y_data_mean: List[float] = field(init=False)

    def __post_init__(self):
        """Compute means."""
        self.y_data_mean = [statistics.mean(subserie)
                            for subserie in self.y_data]


def extract_data_sequence(property_name: Property,
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
    return Sequence("", x_data, y_data)


def nested_dict():
    return defaultdict(nested_dict)


class Plots:
    """."""

    sequences: Dict[Property,
                    Dict[ScenarioIndex,
                         Dict[GroupName,
                              Dict[TcId, Sequence]]]] = nested_dict()

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

    def data_sequence_exists(self,
                             property_name: Property,
                             group_name: GroupName,
                             scenario_index: ScenarioIndex,
                             tc_id: TcId):
        """."""
        sequences = self.sequences
        if property_name not in sequences:
            return False
        if scenario_index not in sequences[property_name]:
            return False
        if group_name not in sequences[property_name][scenario_index]:
            return False
        if tc_id not in sequences[property_name][scenario_index][group_name]:
            return False
        return True

    def get_data_sequence(self,
                          property_name: Property,
                          group_name: GroupName,
                          scenario_index: ScenarioIndex,
                          tc_id: TcId):
        """."""
        return self.sequences[property_name][scenario_index][group_name][tc_id]

    def add_data_sequence(self,
                          property_name: Property,
                          group_name: GroupName,
                          scenario_index: ScenarioIndex,
                          tc_id: TcId,
                          sequence: Sequence):
        """."""
        self.sequences[property_name][scenario_index][group_name][tc_id] = sequence

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
                if not self.data_sequence_exists(property_name,
                                                 group_name,
                                                 scenario_index,
                                                 tc_id):
                    sequence = extract_data_sequence(property_name,
                                                     group_name,
                                                     tc_id,
                                                     scenario_data)
                    self.add_data_sequence(property_name,
                                           group_name,
                                           scenario_index,
                                           tc_id,
                                           sequence)
                else:
                    sequence = self.get_data_sequence(property_name,
                                                      group_name,
                                                      scenario_index,
                                                      tc_id)

    def plot_normal(self,
                    scenarios: List[Tuple[ScenarioIndex, Property]],
                    group_name: GroupName,
                    traffic_classes: List[TcId]):
        """."""
        self.prepare_data(scenarios, group_name, traffic_classes)
        pprint(list(describe_dict(self.sequences)))
