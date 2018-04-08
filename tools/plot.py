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
            [--quiet]
            [--bp] [-i INDICES]
            [-r]
            [--relative-sums]
            [--relative-divs]
            [--normal]
            [-n NAME]
            [--width W] [--height H]
            [--pairs PAIRS]
            [--groups GROUPS]
            [--tc TCs]
            [--title-suffix TITLE_SUFFIX]
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
    -q, --quiet                 don't show plot window
    -d DIR, --output-dir=DIR    directory where the files are saved
                                [default: data/results/plots/]
    --bp                        enable box plots
    -i INDICES                  indices of scenarios to plot [default: -1]
    -r, --relatives             plot relations
    --normal                    normal plots
    --relative-sums             plots of relatives sums
    --relative-divs             plots of relatives divisions
    -n NAME, --name=NAME        suffix added to filename
    --width W                   width of generated image [default: 32]
    --height H                  height of generated image [default: 18]
    --pairs PAIRS               list of scenario pairs for relative plots
    -g GROUPS, --groups=GROUPS  groups that have to be plotted
    --title-suffix=TITLE_SUFFIX title suffix [default: ""]
    --tc=TCs                    filter TCs [default: None]

"""
import os.path as path
import os
import json
import ubjson
import cbor2
import statistics
import itertools
import ast
from pprint import pprint
from docopt import docopt
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
import numpy as np
import scipy as sp
import scipy.stats


stats_name2label = {'served_u': 'Carried traffic',
                    'served': 'Carried requests',
                    'P_block': 'Blocking probability'}


def get_traffic_classes_sizes(scenario):
    tc_sizes = {}
    for tc_id, tc_data in scenario["traffic_classes"].items():
        if tc_id[0] == "_":
            continue
        tc_sizes[int(tc_id)] = tc_data["size"]
    return (tc_sizes, scenario)


def load_traffic_classes_sizes(scenario_file):
    return get_traffic_classes_sizes(json.load(open(scenario_file)))


def append_tc_stat_for_groups_by_size(tc_data_y,
                                      scenario_result,
                                      stat_name,
                                      tc_sizes):
    for group_name, tcs_stats in scenario_result.items():
        if group_name.startswith("_"):
            continue
        group_y = tc_data_y.setdefault(group_name, {})
        new_data = {}
        for tc_id, tc_stats in tcs_stats.items():
            size = tc_sizes[int(tc_id)]
            new_data.setdefault(size, 0)
            new_data[size] += statistics.mean(tc_stats[stat_name])

        for tc_size, data in new_data.items():
            tc_series = group_y.setdefault(tc_size, [])
            tc_series.append(data)


def append_tc_stat_for_groups(tc_data_y,
                              scenario_result,
                              stat_name,
                              tcs_served_by_groups):
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


def set_plot_style(ax):
    ax.grid(axis='both', which='major', linestyle='-')
    ax.grid(axis='x', which='minor', linestyle='--')
    minorLocator = AutoMinorLocator()
    ax.xaxis.set_minor_locator(minorLocator)


def set_plot_linear_style(ax, top=15e5, bottom=0):
    set_plot_style(ax)
    ax.set_yscale("linear")
    ax.set_ylim(bottom=bottom, top=top, auto=True, emit=True)


def set_plot_log_style(ax, top=5, bottom=1e-6):
    set_plot_style(ax)
    ax.set_yscale("log")
    ax.set_ylim(bottom=bottom, top=top, auto=True, emit=True)


def get_tcs_served_by_groups(scenario_results, filter):
    tcs_served_by_groups = {}
    for _, result in scenario_results.items():
        for group_name, tcs_stats in result.items():
            if group_name.startswith("_") or tcs_stats is None:
                continue
            tcs_served_by_group = tcs_served_by_groups.setdefault(
                group_name, [])
            for tc_id, _ in tcs_stats.items():
                if tc_id not in tcs_served_by_group:
                    if filter is None or int(tc_id) in filter:
                        tcs_served_by_group.append(tc_id)
    for _, tcs in tcs_served_by_groups.items():
        tcs.sort()
    return tcs_served_by_groups


def describe_dict(d, prefix=()):
    for k, v in d.items():
        path = prefix + (k, )
        yield path
        if isinstance(v, dict):
            yield from describe_dict(v, path)


def compare_dicts_structure(d1, d2):
    #  pprint(sorted(describe_dict(d1)))
    #  pprint(sorted(describe_dict(d2)))
    return sorted(describe_dict(d1)) == sorted(describe_dict(d2))


def confidence_interval(data, confidence=0.95):
    a = 1.0*np.array(data)
    n = len(a)
    se = scipy.stats.sem(a)
    h = se * sp.stats.t._ppf((1+confidence)/2., n-1)
    return h


def main():
    args = docopt(__doc__, version='0.1')
    data_file = args["<DATA_FILE>"]
    ext = path.splitext(data_file)[1]
    with open(data_file, 'rb') as fp:
        if ext == ".json":
            data = json.load(fp)
        elif ext == ".cbor":
            data = cbor2.load(fp)
        elif ext == ".ubjson":
            data = ubjson.load(fp)
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

    title = path.splitext(path.basename(data_file))[0] + "_" + stat_name
    if args['--relative-divs']:
        title += "_relative_div"
    if args['--relative-sums']:
        title += "_relative_sum"
    if args['--relatives']:
        title += "_relatives"
    if args['--name']:
        title += args['--name']

    fig = plt.figure(
        figsize=(float(args["--width"]), float(args["--height"])), tight_layout=True)
    fig.canvas.set_window_title(title)
    plot_id = 1

    plots_number_x = int(args["-x"])
    plots_number_y = int(args["-y"])

    print("All scenarios:")
    pprint(list(enumerate(data.keys())))

    if args["-i"] == "-1":
        filtered_data = data
    else:
        indices = map(int, args["-i"].split(','))
        filtered_data = {k: data[k]
                         for k in [sorted(data.keys())[i] for i in indices]}

    print("Filtered scenarios:")
    pprint(list(enumerate(filtered_data.keys())))

    all_data = {}

    for scenario_file, scenario_results in filtered_data.items():
        all_data[scenario_file] = {}
        if "_scenario" in scenario_results.keys():
            tc_sizes, scenario = get_traffic_classes_sizes(
                scenario_results["_scenario"])
            scenario_results.pop('_scenario', None)
        else:
            tc_sizes, scenario = load_traffic_classes_sizes(scenario_file)

        scenario['name'] = scenario["name"].replace(";", "\n")
        print(scenario_file)
        tc_data_x = all_data[scenario_file].setdefault("x", [])
        tc_data_y = all_data[scenario_file].setdefault("y", {})
        tc_data_y_by_size = {}
        if not logarithmic_plot:
            def set_style(ax):
                return set_plot_linear_style(ax, y_max, y_min)
            aggregate = True
            if args['--normal']:
                plots_number_x = 2 * len(filtered_data)
        else:
            def set_style(ax):
                return set_plot_log_style(ax, y_max, y_min)
            aggregate = False

        markers = ['+', 'x', 's']

        tcs_served_by_groups = get_tcs_served_by_groups(
            scenario_results, tc_filter)

        for _, result in scenario_results.items():
            # TODO(PW): make this range properly work on different intensity
            # multipliers
            a = float(result["_a"])
            #  if (x_min and x_min > a) or (x_max and x_max < a):
                #  continue
            tc_data_x.append(a)
            append_tc_stat_for_groups(
                tc_data_y, result, stat_name, tcs_served_by_groups)
            if aggregate:
                append_tc_stat_for_groups_by_size(
                    tc_data_y_by_size, result, stat_name, tc_sizes)

        if args['--normal']:
            for group_name, group_data_y in tc_data_y_by_size.items():
                if groups and group_name not in groups:
                    continue
                markerscycle = itertools.cycle(markers)
                ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
                for tc_size, data_y in group_data_y.items():
                    ax.set_xlim(x_min, x_max)
                    ax.plot(tc_data_x, data_y, label="t={}".format(tc_size),
                            marker=next(markerscycle))

                set_style(ax)
                ax.set_title("{} ({})".format(group_name, scenario["name"]))
                ax.set_ylabel(stats_name2label.get(stat_name, stat_name))
                if plot_id % plots_number_x == 0:
                    ax.set_xlabel("a")
                plot_id += 1
                ax.legend(loc=4, ncol=3)

            for group_name, group_data_y in tc_data_y.items():
                if groups and group_name not in groups:
                    continue
                markerscycle = itertools.cycle(markers)
                ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
                #  pprint(tc_data_x)
                for tc_id, data_y in group_data_y.items():

                    if enable_boxplots:
                        ax.set_xlim(x_min, x_max)
                        ax.boxplot(data_y, positions=tc_data_x, notch=False,
                                   widths=0.05, bootstrap=10000, showfliers=False,
                                   vert=True, patch_artist=False, showcaps=False,
                                   whis=0, manage_xticks=False)

                    #  pprint([(tc_id, statistics.mean(serie), confidence_interval(serie))
                        #  for serie in data_y])
                    ax.set_xlim(x_min, x_max)
                    ax.plot(tc_data_x,
                            [statistics.mean(serie) for serie in data_y],
                            #  label="TC{} t={}".format(tc_id, tc_sizes[tc_id]),
                            label="$t_{}={}$".format(tc_id, tc_sizes[tc_id]),
                            marker=next(markerscycle))

                set_style(ax)
                #  ax.set_title("{} V={} {}"
                #  .format(group_name,
                #  scenario["groups"][group_name]["capacity"],
                ax.set_title("{} {}"
                             .format(group_name,
                                     scenario["name"] if title_suffix == None else title_suffix))
                ax.set_ylabel(stats_name2label.get(stat_name, stat_name))
                if plot_id % plots_number_x == 0:
                    ax.set_xlabel("a")
                plot_id += 1
                ax.legend(loc=4, ncol=3)

    if args["--pairs"]:
        key_pairs_id = list(ast.literal_eval(args["--pairs"]))
        keys = list(all_data.keys())
        key_pairs = list(map(lambda p: (
            keys[p[0]], keys[p[1]], p[2] if len(p) > 2 else None), key_pairs_id))
    else:
        key_pairs = itertools.combinations(all_data.keys(), 2)
        key_pairs = list(p + (None,) for p in key_pairs)

    if args['--relatives']:
        for k1, k2, p_name in key_pairs:
            print((k1, k2))
            if all_data[k1]['x'] == all_data[k2]['x']:
                print("OK")
            if not compare_dicts_structure(all_data[k1]['y'], all_data[k2]['y']):
                print("NOT OK")
                #  continue
            for group_name, k1_group_data_y in all_data[k1]['y'].items():
                if groups and group_name not in groups:
                    continue
                k2_group_data_y = all_data[k2]['y'][group_name]

                markerscycle = itertools.cycle(markers)
                ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
                for tc_id, k1_data_y in k1_group_data_y.items():
                    if tc_id not in k2_group_data_y:
                        continue
                    k2_data_y = k2_group_data_y[tc_id]
                    k2_data_y_means = [statistics.mean(x) for x in k2_data_y]
                    k1_data_y_means = [statistics.mean(x) for x in k1_data_y]

                    plot_data = [x/y*100 if y != 0 else 0
                                 for x, y in zip(k1_data_y_means, k2_data_y_means)]

                    label = "$t_{}={}$".format(tc_id, tc_sizes[tc_id])
                    #  label = "TC{} t={}".format(tc_id, tc_sizes[tc_id])
                    data_x = np.array([all_data[k1]['x'], all_data[k2]['x']])
                    #  ax.plot(tc_data_x,
                    ax.set_xlim(x_min, x_max)
                    ax.plot(np.average(data_x, axis=0),
                            plot_data,
                            label=label,
                            marker=next(markerscycle))

                set_style(ax)

                def remove_prefix(s):
                    return s.replace("data/journal/", "")
                if p_name:
                    title_append = p_name
                else:
                    title_append = "\n({} / \n{})".format(
                        remove_prefix(k1), remove_prefix(k2))

                #  ax.set_title("{} V={} {}"
                    #  .format(group_name,
                    #  scenario["groups"][group_name]["capacity"],
                ax.set_title("{} {}"
                             .format(group_name,
                                     title_append))
                ax.set_ylabel("{}\n ratio [%]".format(
                    stats_name2label.get(stat_name, stat_name)))
                if plot_id % plots_number_x == 0:
                    ax.set_xlabel("a")
                plot_id += 1
                ax.legend(loc=9, ncol=5, borderaxespad=0)

    if args['--relative-sums']:
        ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
        markerscycle = itertools.cycle(markers)
        for k1, k2, p_name in key_pairs:
            print((k1, k2))
            if all_data[k1]['x'] == all_data[k2]['x']:
                print("OK")
            if not compare_dicts_structure(all_data[k1]['y'], all_data[k2]['y']):
                print("NOT OK")
                #  continue
            k1_sum = []
            k2_sum = []
            for group_name, k1_group_data_y in all_data[k1]['y'].items():
                if groups and group_name not in groups:
                    continue
                k2_group_data_y = all_data[k2]['y'][group_name]

                for tc_id, k1_data_y in k1_group_data_y.items():
                    k1_data_y_means = [statistics.mean(x) for x in k1_data_y]
                    k1_sum = [
                        x+y for x, y in itertools.zip_longest(k1_sum, k1_data_y_means, fillvalue=0)]
                for tc_id, k2_data_y in k2_group_data_y.items():
                    k2_data_y_means = [statistics.mean(x) for x in k2_data_y]
                    k2_sum = [
                        x+y for x, y in itertools.zip_longest(k2_sum, k2_data_y_means, fillvalue=0)]

            plot_data = [x-y for x, y in zip(k1_sum, k2_sum)]

            def clean(s):
                s.replace("data/journal/", "").replace(".json", "")
            label = "{} - {}".format(clean(k1), clean(k2)
                                     ) if not p_name else p_name
            data_x = np.array([all_data[k1]['x'], all_data[k2]['x']])
            #  ax.plot(tc_data_x,
            ax.set_xlim(x_min, x_max)
            ax.plot(np.average(data_x, axis=0),
                    plot_data,
                    label=label,
                    marker=next(markerscycle))

            set_style(ax)
            #  ax.set_title("Differences of aggregated statistic")
            ax.set_ylabel("{} difference".format(
                stats_name2label.get(stat_name, stat_name)))
            if plot_id % plots_number_x == 0:
                ax.set_xlabel("a")

            plot_id += 1
            ax.legend(loc=9, ncol=2, borderaxespad=0)

    if args['--relative-divs']:
        ax = fig.add_subplot(plots_number_x, plots_number_y, plot_id)
        markerscycle = itertools.cycle(markers)
        for k1, k2, p_name in key_pairs:
            print((k1, k2))
            if all_data[k1]['x'] == all_data[k2]['x']:
                print("X: OK")
            else:
                print("X: NOT OK")
                #  pprint((all_data[k1]['x'], all_data[k2]['x']))

            if not compare_dicts_structure(all_data[k1]['y'], all_data[k2]['y']):
                print("Y: NOT OK")

            k1_sum = []
            k2_sum = []
            for group_name, k1_group_data_y in all_data[k1]['y'].items():
                if groups and group_name not in groups:
                    continue
                k2_group_data_y = all_data[k2]['y'][group_name]

                for tc_id, k1_data_y in k1_group_data_y.items():
                    k1_data_y_means = [statistics.mean(x) for x in k1_data_y]
                    k1_sum = [
                        x+y for x, y in itertools.zip_longest(k1_sum, k1_data_y_means, fillvalue=0)]
                for tc_id, k2_data_y in k2_group_data_y.items():
                    k2_data_y_means = [statistics.mean(x) for x in k2_data_y]
                    k2_sum = [
                        x+y for x, y in itertools.zip_longest(k2_sum, k2_data_y_means, fillvalue=0)]

            plot_data = [x/y*100 if y != 0 else 0 for x,
                         y in zip(k1_sum, k2_sum)]

            def clean(s):
                return s.replace("data/journal/", "").replace(".json", "")
            label = "{} - {}".format(clean(k1), clean(k2)
                                     ) if not p_name else p_name
            data_x = np.array([all_data[k1]['x'], all_data[k2]['x']])
            #  ax.plot(tc_data_x,
            #  pprint(np.average(data_x, axis=0))
            ax.set_xlim(x_min, x_max)
            ax.plot(np.average(data_x, axis=0),
                    plot_data,
                    label=label,
                    marker=next(markerscycle))

            set_style(ax)
            if title_suffix:
                ax.set_title(title_suffix)
            #  ax.set_title("Differences of aggregated statistic")
            ax.set_ylabel("{} ratio [%]".format(
                stats_name2label.get(stat_name, stat_name)))
            if plot_id % plots_number_x == 0:
                ax.set_xlabel("a")
            plot_id += 1
            ax.legend(loc=9, ncol=2, borderaxespad=0)

    if not args["--quiet"]:
        plt.show()

    if args["--save"]:
        output_dir = args["--output-dir"]
        if not os.path.isdir(output_dir):
            os.makedirs(output_dir)
        output_file = title + ".pdf"
        output_file = path.join(output_dir, output_file)
        fig.set_size_inches(float(args["--width"]), float(args["--height"]))
        fig.savefig(output_file)


if __name__ == "__main__":
    main()
