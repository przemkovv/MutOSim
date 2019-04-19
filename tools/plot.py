#!/usr/bin/env python

"""
Simulator of network with Mutual Overflows.

Usage:
    plot.py <DATA_FILE> [-p PROPERTY]
            [--linear]
            [--y_max=Y_MAX] [--y_min=Y_MIN]
            [--x_max=X_MAX] [--x_min=X_MIN]
            [-x X] [-y Y]
            [--save] [--output-dir=DIR]
            [--format=FORMAT]
            [--quiet]
            [--bp] [-i INDICES]
            [-r] [--relatives]
            [--relatives-diffs]
            [--relative-sums]
            [--relative-divs]
            [--normal]
            [-n NAME]
            [--width W] [--height H]
            [--pairs PAIRS]
            [--groups GROUPS]
            [--tc TCs]
            [--title-suffix TITLE_SUFFIX]
            [--no-pair-suffix]
            [--print-all]
    plot.py -h | --help

Arguments:
    DATA_FILE   path to the file with data to plot

Options:
    -h --help                   show this help message and exit
    -p PROPERTY                 property from data file to plot
                                [default: P_block]
    --linear                    linear plot (default is log)
    --x_max=X_MAX               right limit on x axis [default: 3.0]
    --x_min=X_MIN               left limit on x axis [default: 0.0]
    --y_max=Y_MAX               top limit on y axis [default: 5]
    --y_min=Y_MIN               bottom limit on y axis [default: 1e-6]
    -x X                        number of plots horizontally [default: 3]
    -y Y                        number of plots vertically [default: 3]
    -s, --save                  save to file
    --format FORMAT             output format [default: pdf]
    -q, --quiet                 don't show plot window
    -d DIR, --output-dir=DIR    directory where the files are saved
                                [default: data/results/plots/]
    --bp                        enable box plots
    -i INDICES                  indices of scenarios to plot [default: -1]
    -r, --relatives             plot relations (ratio)
    --relatives-diffs           plot relations (difference)
    --normal                    normal plots
    --relative-sums             plots of relatives sums
    --relative-divs             plots of relatives divisions
    -n NAME, --name=NAME        suffix added to filename
    --width W                   width of generated image [default: 32]
    --height H                  height of generated image [default: 18]
    --pairs PAIRS               list of scenario pairs for relative plots
    --no-pair-suffix            skip automatic suffix for pairs
    --print-all                 print all scenarios with ids
    -g GROUPS, --groups=GROUPS  groups that have to be plotted
    --title-suffix=TITLE_SUFFIX title suffix [default: ""]
    --tc=TCs                    filter TCs [default: None]

"""
import os
import json
import math
import statistics
import itertools
import ast
from pprint import pprint
from typing import Optional
from typing import List
from typing import Iterable
from docopt import docopt
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
from matplotlib.pyplot import Axes
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
}

YELLOW = f"{Fore.YELLOW}{Style.BRIGHT}"
BLUE = f"{Fore.BLUE}{Style.BRIGHT}"
GREEN = f"{Fore.GREEN}"


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


def set_plot_linear_style(axes: Axes, top=15e5, bottom=0):
    """Set linear plot style."""
    set_plot_style(axes)
    axes.set_yscale("linear")
    axes.set_ylim(bottom=bottom, top=top, auto=True, emit=True)


def set_plot_log_style(axes: Axes, top=5, bottom=1e-6):
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


def describe_dict(dictionary: dict, prefix=()):
    """Return an hierarchical structure of the dictionary."""
    for key, value in dictionary.items():
        path = prefix + (key,)
        yield path
        if isinstance(value, dict):
            yield from describe_dict(value, path)


def compare_dicts_structure(dict1, dict2):
    """Compare if two dictionaries structures matches themselves."""
    #  pprint(sorted(describe_dict(dict1)))
    #  pprint(sorted(describe_dict(dict2)))
    return sorted(describe_dict(dict1)) == sorted(describe_dict(dict2))


def confidence_interval(data, confidence=0.95):
    """Return a values interval for a given confidence."""
    a = 1.0 * np.array(data)
    n = len(a)
    se = scipy.stats.sem(a)
    h = se * sp.stats.t._ppf((1 + confidence) / 2.0, n - 1)
    return h


def clean(s: str):
    """Remove prefixes from the name."""
    return (
        s.replace("data/journal/", "")
        .replace("data/journal2/", "")
        .replace("data/scenarios/simulator_publication/", "")
        .replace("data/scenarios/analytical/layer_types/", "")
        .replace(".json", "")
    )


def main():
    """."""
    args = docopt(__doc__, version="0.1")
    data_filename = args["<DATA_FILE>"]
    ext = os.path.splitext(data_filename)[1]
    with open(data_filename, "rb") as data_file:
        if ext == ".json":
            data = json.load(data_file)
        elif ext == ".cbor":
            data = cbor2.load(data_file)
        elif ext == ".ubjson":
            data = ubjson.load(data_file)
    stat_name = args["-p"]
    x_min = float(args["--x_min"])
    x_max = float(args["--x_max"])
    y_min = float(args["--y_min"])
    y_max = float(args["--y_max"])
    logarithmic_plot = not args["--linear"]
    enable_boxplots = args["--bp"]
    title_suffix = args["--title-suffix"]

    groups = ast.literal_eval(args["--groups"]) if args["--groups"] else None

    tc_filter = ast.literal_eval(args["--tc"]) if args["--tc"] else []

    title = os.path.splitext(os.path.basename(data_filename))[
        0] + "_" + stat_name
    if args["--relative-divs"]:
        title += "_relative_div"
    if args["--relative-sums"]:
        title += "_relative_sum"
    if args["--relatives"]:
        title += "_relatives"
    if args["--name"]:
        title += args["--name"]

    #  rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
    # for Palatino and other serif fonts use:
    # rc('font',**{'family':'serif','serif':['Palatino']})
    #  rc('text', usetex=True)

    plots_number_x = int(args["-x"])
    plots_number_y = int(args["-y"])

    fig = plt.figure(
        figsize=(float(args["--width"]), float(args["--height"])), tight_layout=True
    )
    fig.canvas.set_window_title(title)
    plot_id = 1
    glob_fig = plt.figure(
        figsize=(
            float(args["--width"]) / plots_number_y,
            float(args["--height"]) / plots_number_x,
        ),
        tight_layout=True,
    )
    glob_fig.canvas.set_window_title(title)

    glob_ax = glob_fig.add_subplot(1, 1, 1)

    if args["--print-all"]:
        print("All scenarios:")
        pprint(list(enumerate(data.keys())))

    if args["-i"] == "-1":
        filtered_data = data
    else:
        indices = map(int, args["-i"].split(","))
        filtered_data = {k: data[k] for k in [
            sorted(data.keys())[i] for i in indices]}

    print(f"{YELLOW}Filtered scenarios:")
    pprint(list(enumerate(filtered_data.keys())))

    all_data = {}
    scenario_id = -1

    for scenario_file, scenario_results in filtered_data.items():
        scenario_id = scenario_id + 1
        all_data[scenario_file] = {}
        if "_scenario" in scenario_results.keys():
            tc_sizes, scenario = get_traffic_classes_sizes(
                scenario_results["_scenario"]
            )
            scenario_results.pop("_scenario", None)
        else:
            raise ValueError("Scenario data are missing.")

        scenario["name"] = scenario["name"].replace(";", "\n")
        print(f"{GREEN}{scenario_file}")
        tc_data_x = all_data[scenario_file].setdefault("x", [])
        tc_data_y = all_data[scenario_file].setdefault("y", {})
        tc_data_y_by_size = {}
        if not logarithmic_plot:
            def set_style(axes: Axes):
                return set_plot_linear_style(axes, y_max, y_min)

            aggregate = True
            if args["--normal"]:
                plots_number_x = 2 * len(filtered_data)
        else:
            def set_style(axes: Axes):
                return set_plot_log_style(axes, y_max, y_min)

            aggregate = False

        markers = ["+", "x", "2"]
        glob_markers = ["+", "x", "2"]
        glob_linestyle = [":", "-"]

        tcs_served_by_groups = get_tcs_served_by_groups(
            scenario_results, tc_filter)

        for _, result in scenario_results.items():
            # TODO(PW): make this range properly work on different intensity
            # multipliers
            a = float(result["_A"])
            #  if (x_min and x_min > a) or (x_max and x_max < a):
            #  continue
            tc_data_x.append(a)
            append_tc_stat_for_groups(
                tc_data_y, result, stat_name, tcs_served_by_groups
            )
            if aggregate:
                append_tc_stat_for_groups_by_size(
                    tc_data_y_by_size, result, stat_name, tc_sizes
                )

        if args["--normal"]:
            for group_name, group_data_y in tc_data_y_by_size.items():
                if not get_valid_group(group_name, groups):
                    continue

                markerscycle = itertools.cycle(markers)
                glob_markerscycle = itertools.cycle(glob_markers)
                ax = fig.add_subplot(
                    plots_number_x, plots_number_y, plot_id)
                for tc_size, data_y in group_data_y.items():
                    ax.set_xlim(x_min, x_max)
                    ax.plot(
                        tc_data_x,
                        data_y,
                        label="t={}".format(tc_size),
                        marker=next(markerscycle),
                    )
                    glob_ax.set_xlim(x_min, x_max)
                    glob_ax.plot(
                        tc_data_x,
                        data_y,
                        label="t={}".format(tc_size),
                        marker=next(glob_markerscycle),
                    )

                set_style(ax)
                set_style(glob_ax)
                ax.set_title(
                    "{} ({}\n{})".format(group_name,
                                         scenario["name"], scenario_file)
                )
                ax.set_ylabel(STAT_NAME_TO_LABEL.get(
                    stat_name, stat_name))
                glob_ax.set_title(
                    "{} ({}\n{})".format(group_name,
                                         scenario["name"], scenario_file)
                )
                glob_ax.set_ylabel(
                    STAT_NAME_TO_LABEL.get(stat_name, stat_name))
                if plot_id % plots_number_x == 0:
                    ax.set_xlabel("a")
                plot_id += 1
                ax.legend(loc=4, ncol=3)
                glob_ax.legend(loc=4, ncol=3)

            for group_name, group_data_y in tc_data_y.items():
                if not get_valid_group(group_name, groups):
                    continue

                markerscycle = itertools.cycle(markers)
                glob_markerscycle = itertools.cycle(glob_markers)
                ax = fig.add_subplot(
                    plots_number_x, plots_number_y, plot_id)
                #  pprint(tc_data_x)
                for tc_id, data_y in group_data_y.items():

                    if enable_boxplots:
                        ax.set_xlim(x_min, x_max)
                        ax.boxplot(
                            data_y,
                            positions=tc_data_x,
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

                    #  pprint([(tc_id, statistics.mean(serie), confidence_interval(serie))
                    #  for serie in data_y])
                    ax.set_xlim(x_min, x_max)
                    glob_ax.set_xlim(x_min, x_max)
                    #  pprint((len(tc_data_x), tc_data_x,
                    #  len([statistics.mean(serie) for serie in data_y]),
                    #  [statistics.mean(serie) for serie in data_y]))

                    series_means = [statistics.mean(
                        serie) for serie in data_y]
                    for i in range(0, len(tc_data_x) - len(series_means)):
                        series_means.insert(0, 0)
                    ax.plot(
                        tc_data_x,
                        series_means,
                        #  label="TC{} t={}".format(tc_id, tc_sizes[tc_id]),
                        label="$t_{}={}$".format(
                            tc_id, tc_sizes[tc_id]),
                        marker=next(markerscycle),
                    )
                    glob_ax.plot(
                        tc_data_x,
                        series_means,
                        #  label="TC{} t={}".format(tc_id, tc_sizes[tc_id]),
                        label="{} $t_{}={}$".format(
                            scenario_id, tc_id, tc_sizes[tc_id]
                        ),
                        linestyle=glob_linestyle[scenario_id],
                        markevery=5,
                        marker=next(glob_markerscycle),
                    )

                set_style(ax)
                set_style(glob_ax)
                #  ax.set_title("{} V={} {}"
                #  .format(group_name,
                #  scenario["groups"][group_name]["capacity"],
                if title_suffix == None:
                    ax.set_title(
                        "{} {}\n({})".format(
                            group_name, scenario["name"], scenario_file
                        )
                    )
                else:
                    ax.set_title("{} {}".format(
                        group_name, title_suffix))
                ax.set_ylabel(STAT_NAME_TO_LABEL.get(
                    stat_name, stat_name))
                glob_ax.set_ylabel(
                    STAT_NAME_TO_LABEL.get(stat_name, stat_name))
                if plot_id % plots_number_x == 0:
                    ax.set_xlabel("a")
                plot_id += 1
                ax.legend(loc=4, ncol=3)
                glob_ax.legend(loc=4, ncol=3)

    if args["--pairs"]:
        key_pairs_id = list(ast.literal_eval(args["--pairs"]))
        keys = list(all_data.keys())
        key_pairs = list(
            map(
                lambda p: (keys[p[0]], keys[p[1]], p[2]
                           if len(p) > 2 else None),
                key_pairs_id,
            )
        )
    else:
        key_pairs = itertools.combinations(all_data.keys(), 2)
        key_pairs = list(p + (None,) for p in key_pairs)

    if args["--relatives-diffs"]:
        for k1, k2, p_name in key_pairs:
            print((k1, k2))
            if all_data[k1]["x"] == all_data[k2]["x"]:
                print("OK")
            if not compare_dicts_structure(all_data[k1]["y"], all_data[k2]["y"]):
                print("NOT OK")

            for group_name, k1_group_data_y in all_data[k1]["y"].items():
                valid_group_k1 = get_valid_group(group_name, groups)

                if not valid_group_k1:
                    continue
                valid_group_k2 = get_corresponding_group(
                    valid_group_k1, all_data[k2]["y"].keys()
                )
                if not valid_group_k2:
                    continue

                k2_group_data_y = all_data[k2]["y"][valid_group_k2]

                markerscycle = itertools.cycle(markers)
                ax = fig.add_subplot(
                    plots_number_x, plots_number_y, plot_id)
                for tc_id, k1_data_y in k1_group_data_y.items():
                    if tc_id not in k2_group_data_y:
                        continue
                    k2_data_y = k2_group_data_y[tc_id]
                    k2_data_y_means = [
                        statistics.mean(x) for x in k2_data_y]
                    k1_data_y_means = [
                        statistics.mean(x) for x in k1_data_y]

                    plot_data = [
                        x - y for x, y in zip(k1_data_y_means, k2_data_y_means)
                    ]
                    #  pprint((len(k1_data_y_means), len(k2_data_y_means)))

                    label = "$t_{}={}$".format(tc_id, tc_sizes[tc_id])
                    #  label = "TC{} t={}".format(tc_id, tc_sizes[tc_id])
                    data_x = np.array(
                        [all_data[k1]["x"], all_data[k2]["x"]])
                    #  pprint((k1, len(all_data[k1]['x']), k2, len(all_data[k2]['x'])))
                    #  ax.plot(tc_data_x,
                    #  pprint((len(data_x), len(data_x[0]), data_x))
                    #  pprint((len(plot_data), plot_data))
                    for i in range(0, len(data_x[0]) - len(plot_data)):
                        plot_data.insert(0, 0)
                    #  pprint((k1, len(all_data[k1]['x']), k2, len(all_data[k2]['x']), label))
                    #  pprint(("ZIP", list(zip(data_x[0], k1_data_y_means, k2_data_y_means))))
                    #  pprint(plot_data)

                    ax.set_xlim(x_min, x_max)

                    ax.plot(
                        np.average(data_x, axis=0),
                        plot_data,
                        label=label,
                        marker=next(markerscycle),
                    )

                set_style(ax)

                def remove_prefix(s):
                    return (
                        s.replace("data/journal/", "")
                        .replace("data/journal2/", "")
                        .replace(".json", "")
                    )

                if p_name:
                    title_append = p_name
                else:
                    title_append = "\n({} - \n{})".format(
                        remove_prefix(k1), remove_prefix(k2)
                    )

                #  ax.set_title("{} V={} {}"
                #  .format(group_name,
                #  scenario["groups"][group_name]["capacity"],
                #  ax.set_title("{} {}"
                #  .format(group_name,
                #  title_append))
                ax.set_title(
                    "{} {}".format(
                        group_name,
                        title_append if title_suffix == None else title_suffix,
                    )
                )
                ax.set_ylabel(
                    "{}\n difference".format(
                        STAT_NAME_TO_LABEL.get(stat_name, stat_name)
                    )
                )
                if plot_id % plots_number_x == 0:
                    ax.set_xlabel("a")
                plot_id += 1
                handles, labels = ax.get_legend_handles_labels()
                ncol = cols_number(labels)
                ax.legend(
                    flip(handles, ncol),
                    flip(labels, ncol),
                    loc=9,
                    ncol=3,
                    borderaxespad=0,
                )
                #  ax.legend(loc=9, ncol=5, borderaxespad=0)
    if args["--relatives"]:
        for k1, k2, p_name in key_pairs:
            print((k1, k2))
            if all_data[k1]["x"] == all_data[k2]["x"]:
                print("OK")
            if not compare_dicts_structure(all_data[k1]["y"], all_data[k2]["y"]):
                print("NOT OK")
                #  continue
            for group_name, k1_group_data_y in all_data[k1]["y"].items():
                if groups and group_name not in groups:
                    continue
                k2_group_data_y = all_data[k2]["y"][group_name]

                markerscycle = itertools.cycle(markers)
                ax = fig.add_subplot(
                    plots_number_x, plots_number_y, plot_id)
                for tc_id, k1_data_y in k1_group_data_y.items():
                    if tc_id not in k2_group_data_y:
                        continue
                    k2_data_y = k2_group_data_y[tc_id]
                    k2_data_y_means = [
                        statistics.mean(x) for x in k2_data_y]
                    k1_data_y_means = [
                        statistics.mean(x) for x in k1_data_y]

                    plot_data = [
                        x / y * 100 if y != 0 else 0
                        for x, y in zip(k1_data_y_means, k2_data_y_means)
                    ]
                    #  pprint((len(k1_data_y_means), len(k2_data_y_means)))

                    label = "$t_{}={}$".format(tc_id, tc_sizes[tc_id])
                    #  label = "TC{} t={}".format(tc_id, tc_sizes[tc_id])
                    data_x = np.array(
                        [all_data[k1]["x"], all_data[k2]["x"]])
                    #  pprint((k1, len(all_data[k1]['x']), k2, len(all_data[k2]['x'])))
                    #  ax.plot(tc_data_x,
                    #  pprint((len(data_x), len(data_x[0]), data_x))
                    #  pprint((len(plot_data), plot_data))
                    for i in range(0, len(data_x[0]) - len(plot_data)):
                        plot_data.insert(0, 0)
                    ax.set_xlim(x_min, x_max)

                    ax.plot(
                        np.average(data_x, axis=0),
                        plot_data,
                        label=label,
                        marker=next(markerscycle),
                    )

                set_style(ax)

                def remove_prefix(s):
                    return (
                        s.replace("data/journal/", "")
                        .replace("data/journal2/", "")
                        .replace(".json", "")
                    )

                if p_name:
                    title_append = p_name
                else:
                    title_append = "\n({} / \n{})".format(
                        remove_prefix(k1), remove_prefix(k2)
                    )

                #  ax.set_title("{} V={} {}"
                #  .format(group_name,
                #  scenario["groups"][group_name]["capacity"],
                #  ax.set_title("{} {}"
                #  .format(group_name,
                #  title_append))
                ax.set_title(
                    "{} {}".format(
                        group_name,
                        title_append if title_suffix == None else title_suffix,
                    )
                )
                ax.set_ylabel(
                    "{}\n ratio [%]".format(
                        STAT_NAME_TO_LABEL.get(stat_name, stat_name)
                    )
                )
                if plot_id % plots_number_x == 0:
                    ax.set_xlabel("a")
                plot_id += 1
                handles, labels = ax.get_legend_handles_labels()
                ncol = cols_number(labels)
                ax.legend(
                    flip(handles, ncol),
                    flip(labels, ncol),
                    loc=9,
                    ncol=ncol,
                    borderaxespad=0,
                )
                #  ax.legend(loc=9, ncol=5, borderaxespad=0)

    if args["--relative-sums"]:
        ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
        markerscycle = itertools.cycle(markers)
        for k1, k2, p_name in key_pairs:
            print((k1, k2))
            if all_data[k1]["x"] == all_data[k2]["x"]:
                print("OK")
            if not compare_dicts_structure(all_data[k1]["y"], all_data[k2]["y"]):
                print("NOT OK")
                #  continue
            k1_sum = []
            k2_sum = []
            for group_name, k1_group_data_y in all_data[k1]["y"].items():
                if groups and group_name not in groups:
                    continue
                k2_group_data_y = all_data[k2]["y"][group_name]

                for tc_id, k1_data_y in k1_group_data_y.items():
                    k1_data_y_means = [
                        statistics.mean(x) for x in k1_data_y]
                    k1_sum = [
                        x + y
                        for x, y in itertools.zip_longest(
                            k1_sum, k1_data_y_means, fillvalue=0
                        )
                    ]
                for tc_id, k2_data_y in k2_group_data_y.items():
                    k2_data_y_means = [
                        statistics.mean(x) for x in k2_data_y]
                    k2_sum = [
                        x + y
                        for x, y in itertools.zip_longest(
                            k2_sum, k2_data_y_means, fillvalue=0
                        )
                    ]

            plot_data = [x - y for x, y in zip(k1_sum, k2_sum)]

            label = "{} - {}".format(clean(k1),
                                     clean(k2)) if not p_name else p_name
            data_x = np.array([all_data[k1]["x"], all_data[k2]["x"]])
            #  ax.plot(tc_data_x,
            ax.set_xlim(x_min, x_max)
            ax.plot(
                np.average(data_x, axis=0),
                plot_data,
                label=label,
                marker=next(markerscycle),
            )

            set_style(ax)
            #  ax.set_title("Differences of aggregated statistic")
            ax.set_ylabel(
                "{} difference".format(
                    STAT_NAME_TO_LABEL.get(stat_name, stat_name))
            )
            if plot_id % plots_number_x == 0:
                ax.set_xlabel("a")

            plot_id += 1
            ax.legend(loc=9, ncol=2, borderaxespad=0)

    if args["--relative-divs"]:
        ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
        markerscycle = itertools.cycle(markers)
        for k1, k2, p_name in key_pairs:
            print((k1, k2))
            if all_data[k1]["x"] == all_data[k2]["x"]:
                print("X: OK")
            else:
                print("X: NOT OK")
                #  pprint((all_data[k1]['x'], all_data[k2]['x']))

            if not compare_dicts_structure(all_data[k1]["y"], all_data[k2]["y"]):
                print("Y: NOT OK")

            k1_sum = []
            k2_sum = []
            for group_name, k1_group_data_y in all_data[k1]["y"].items():
                if groups and group_name not in groups:
                    continue
                k2_group_data_y = all_data[k2]["y"][group_name]

                for tc_id, k1_data_y in k1_group_data_y.items():
                    k1_data_y_means = [
                        statistics.mean(x) for x in k1_data_y]
                    k1_sum = [
                        x + y
                        for x, y in itertools.zip_longest(
                            k1_sum, k1_data_y_means, fillvalue=0
                        )
                    ]
                for tc_id, k2_data_y in k2_group_data_y.items():
                    k2_data_y_means = [
                        statistics.mean(x) for x in k2_data_y]
                    k2_sum = [
                        x + y
                        for x, y in itertools.zip_longest(
                            k2_sum, k2_data_y_means, fillvalue=0
                        )
                    ]

                plot_data = [
                    x / y * 100 if y != 0 else 0 for x, y in zip(k1_sum, k2_sum)
                ]

                label = (
                    "{} - {} ".format(clean(k1), clean(k2))
                    if p_name is None
                    else p_name
                )
                if not args["--no-pair-suffix"]:
                    label = "{}{}".format(label, group_name)
                data_x = np.array(
                    [all_data[k1]["x"], all_data[k2]["x"]])
                #  ax.plot(tc_data_x,
                #  pprint(np.average(data_x, axis=0))
                ax.set_xlim(x_min, x_max)
                ax.plot(
                    np.average(data_x, axis=0),
                    plot_data,
                    label=label,
                    marker=next(markerscycle),
                )

            set_style(ax)
            if title_suffix:
                ax.set_title(title_suffix)
            #  ax.set_title("Differences of aggregated statistic")
            ax.set_ylabel(
                "{} ratio [%]".format(
                    STAT_NAME_TO_LABEL.get(stat_name, stat_name))
            )
            if plot_id % plots_number_x == 0:
                ax.set_xlabel("a")
            plot_id += 1
            handles, labels = ax.get_legend_handles_labels()
            ncol = cols_number(labels)
            ax.legend(
                flip(handles, ncol),
                flip(labels, ncol),
                loc=9,
                ncol=ncol,
                borderaxespad=0,
            )
            #  ax.legend(loc=9, ncol=2, borderaxespad=0)

    if not args["--quiet"]:
        plt.show()

    if args["--save"]:

        def create_filename(name: str):
            output_dir = args["--output-dir"]
            if not os.path.isdir(output_dir):
                os.makedirs(output_dir)
            output_file = name + "." + args["--format"]
            output_file = os.path.join(output_dir, output_file)
            return output_file

        output_file = create_filename(title)
        fig.set_size_inches(
            float(args["--width"]), float(args["--height"]))
        print(f"{BLUE}Saving {output_file}")
        fig.savefig(output_file, transparent=True)

        output_file = create_filename("glob_" + title)
        glob_fig.set_size_inches(
            float(args["--width"]) / plots_number_y,
            float(args["--height"]) / plots_number_x,
        )
        print(f"{BLUE}Saving {output_file}")
        glob_fig.savefig(output_file, transparent=True)


if __name__ == "__main__":
    colorama.init(autoreset=True)
    main()
