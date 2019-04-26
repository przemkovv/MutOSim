
import os
import json
import itertools
import math
from typing import Optional, List, Iterable, Dict, Tuple
import ubjson
import cbor2
import numpy as np
import scipy as sp
import scipy.stats
from colorama import Fore, Style

YELLOW = f"{Fore.YELLOW}{Style.BRIGHT}"
BLUE = f"{Fore.BLUE}{Style.BRIGHT}"
GREEN = f"{Fore.GREEN}"
RED = f"{Fore.RED}{Style.BRIGHT}"


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


def load_data2(filename: str) -> Dict[str, Dict[str, dict]]:
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


def remove_prefix(text: str, prefix: str) -> str:
    """."""
    if text.startswith(prefix):
        return text[len(prefix):]
    return text
