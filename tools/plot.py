#!/bin/python
"""Usage:
    plot.py <DATA_FILE> [-p PROPERTY] [--linear] [--y_limit=Y_LIMIT] [-x X] [-y Y]
    plot.py -h | --help

Arguments:
    DATA_FILE   path to the file with data to plot

Options:
    -h --help               show this help message and exit
    -p PROPERTY             property from data file to plot [default: P_block]
    --linear                linear plot (default is log)
    --y_limit=Y_LIMIT       bottom (for log) or top (for linear) limit on y axis [default: 1e-6]
    -x X                    number of plots horizontally [default: 3]
    -y Y                    number of plots vertically [default: 3]

"""
from docopt import docopt
from collections import defaultdict
import json
from pprint import pprint
import matplotlib.pyplot as plt
import scipy.stats as st
import numpy as np
from matplotlib.ticker import AutoMinorLocator

import itertools
import statistics


def load_traffic_classes_sizes(scenario_file):
    scenario = json.load(open(scenario_file))
    tc_sizes = {}
    for tc_id, tc_data in scenario["traffic_classes"].items():
        if tc_id[0] == "_":
            continue
        tc_sizes[int(tc_id)] = tc_data["size"]

    return (tc_sizes, scenario)


def append_tc_stat_for_groups_by_size(tc_data_y, scenario_result, stat_name, tc_sizes):
    for group_name, tcs_stats in scenario_result.items():
        if group_name.startswith("_"):
            continue
        group_y = tc_data_y.setdefault(group_name, {})
        new_data = {}
        for tc_id, tc_stats in tcs_stats.items():
            size = tc_sizes[int(tc_id)]
            #  new_data.setdefault(size, tc_stats[stat_name])
            #  new_data[size] = [sum(x) for x in zip ( new_data[size], tc_stats[stat_name])]
            new_data.setdefault(size, 0)
            new_data[size] += statistics.mean(tc_stats[stat_name])

        for tc_size, data in new_data.items():
            tc_series = group_y.setdefault(tc_size, [])
            tc_series.append(data)


def append_tc_stat_for_groups(tc_data_y, scenario_result, stat_name, tcs_served_by_groups):
    for group_name, tcs_stats in scenario_result.items():
        if group_name.startswith("_"):
            continue
        group_y = tc_data_y.setdefault(group_name, {})
        print(group_name)
        for tc_id in tcs_served_by_groups[group_name]:
            tc_series = group_y.setdefault(int(tc_id), [])
            if tc_id in tcs_stats:
                tc_stats = tcs_stats[tc_id]
                tc_serie = tc_stats[stat_name]
                mean = np.mean(tc_serie)
                tc_series.append(mean)
                if len(tc_serie) > 1:
                    ci_interval = st.t.interval(
                        0.95, len(tc_serie)-1, loc=mean, scale=st.sem(tc_serie))
                    pprint((ci_interval - mean )/mean)
            else:
                tc_series.append(0)

    print("RRR")


def set_plot_style(ax):
    ax.grid(axis='both', which='major', linestyle='-')
    ax.grid(axis='x', which='minor', linestyle='--')
    minorLocator = AutoMinorLocator()
    ax.xaxis.set_minor_locator(minorLocator)


def set_plot_linear_style(ax, top=15e5):
    set_plot_style(ax)
    ax.set_yscale("linear")
    ax.set_ylim(bottom=0, top=top, auto=True, emit=True)


def set_plot_log_style(ax, bottom=1e-6):
    set_plot_style(ax)
    ax.set_yscale("log")
    ax.set_ylim(bottom=bottom, top=5, auto=True, emit=True)


def get_tcs_served_by_groups(scenario_results):
    tcs_served_by_groups = {}
    for _, result in scenario_results.items():
        for group_name, tcs_stats in result.items():
            if group_name.startswith("_"):
                continue
            tcs_served_by_group = tcs_served_by_groups.setdefault(
                group_name, [])
            for tc_id, tc_stats in tcs_stats.items():
                if tc_id not in tcs_served_by_group:
                    tcs_served_by_group.append(tc_id)
    for _, tcs in tcs_served_by_groups.items():
        tcs.sort()
    return tcs_served_by_groups


def main():
    args = docopt(__doc__, version='0.1')
    data = json.load(open(args["<DATA_FILE>"]))
    property = args["-p"]
    y_limit = float(args["--y_limit"])
    logarithmic_plot = not args["--linear"]

    fig = plt.figure(figsize=(32, 18), tight_layout=True)
    plot_id = 1

    plots_number_x = args["-x"]
    plots_number_y = args["-y"]

    for scenario_file, scenario_results in data.items():
        tc_sizes, scenario = load_traffic_classes_sizes(scenario_file)
        print(scenario_file)
        tc_data_x = []
        tc_data_y = {}
        tc_data_y_by_size = {}
        if not logarithmic_plot:
            stat_name = property

            def set_style(ax): return set_plot_linear_style(ax, y_limit)
            aggregate = True
            plots_number_x = 2 * len(data)
        else:
            stat_name = property

            def set_style(ax): return set_plot_log_style(ax, y_limit)
            aggregate = False

        markers = ['+', 'x', 's']

        tcs_served_by_groups = get_tcs_served_by_groups(scenario_results)

        for A, result in scenario_results.items():
            tc_data_x.append(float(result["_a"]))
            append_tc_stat_for_groups(
                tc_data_y, result, stat_name, tcs_served_by_groups)
            if aggregate:
                append_tc_stat_for_groups_by_size(
                    tc_data_y_by_size, result, stat_name, tc_sizes)

        for group_name, group_data_y in tc_data_y_by_size.items():
            markerscycle = itertools.cycle(markers)
            ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
            for tc_size, data_y in group_data_y.items():
                #  ax.boxplot(data_y,positions=tc_data_x, notch=True, widths=0.05,sym='')
                ax.plot(tc_data_x, data_y, label="S{}".format(tc_size),
                        marker=next(markerscycle))

            set_style(ax)
            ax.set_title("{} ({})".format(group_name, scenario["name"]))
            ax.set_ylabel(stat_name)
            ax.set_xlabel("a")
            plot_id += 1
            ax.legend()

        for group_name, group_data_y in tc_data_y.items():
            markerscycle = itertools.cycle(markers)
            ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
            for tc_id, data_y in group_data_y.items():
                #  ax.plot(tc_data_x, data_y, label="TC{} S{}".format(tc_id, tc_sizes[tc_id]),
                           #  marker=next(markerscycle))
                ax.plot(tc_data_x, data_y, label="TC{} S{}".format(tc_id, tc_sizes[tc_id]),
                        marker=next(markerscycle))

            set_style(ax)
            ax.set_title("{} ({})".format(group_name, scenario["name"]))
            ax.set_ylabel(stat_name)
            ax.set_xlabel("a")
            plot_id += 1
            ax.legend()

    #  plt.savefig("test.pdf")
    plt.show()


if __name__ == "__main__":
    main()
