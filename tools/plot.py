
import sys
import json
from pprint import pprint
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
import itertools


def load_traffic_classes_sizes(scenario_file):
    scenario = json.load(open(scenario_file))
    tc_sizes = {}
    for tc_id, tc_data in scenario["traffic_classes"].items():
        if tc_id[0] == "_":
            continue
        tc_sizes[int(tc_id)] = tc_data["size"]

    return (tc_sizes, scenario)


def main(argv):
    print(argv[0])
    data = json.load(open(argv[0]))

    plot_id = 1
    for scenario_file, scenario_results in data.items():
        tc_sizes, scenario = load_traffic_classes_sizes(scenario_file)
        print(scenario_file)
        tc_data_x = []
        tc_data_y = {}
        for A, result in scenario_results.items():
            tc_data_x.append(float(result["_a"]))
            for group_name, tcs_stats in result.items():
                if group_name[0] == "_":
                    continue
                if not group_name in tc_data_y.keys():
                    tc_data_y[group_name] = {}
                for tc_id, tc_stats in tcs_stats.items():
                    if not int(tc_id) in tc_data_y[group_name].keys():
                        tc_data_y[group_name][int(tc_id)] = []
                    tc_data_y[group_name][int(tc_id)].append( tc_stats["P_block"])

        for group_name, group_data_y in tc_data_y.items():
            markers = ['+', 'x', 's']
            #  markers = ['+', 'x', 'D', '*', 'd', 's']
            markerscycle = itertools.cycle(markers)
            ax = plt.subplot(3, 3, plot_id)
            for tc_id, data_y in group_data_y.items():
                ax.scatter(tc_data_x, data_y, label="TC{} S{}".format(tc_id, tc_sizes[tc_id]),
                        marker=next(markerscycle))

            minorLocator = AutoMinorLocator()
            ax.grid(axis='both', which='major', linestyle='-')
            ax.grid(axis='x', which='minor', linestyle='--')
            ax.set_yscale("log")
            ax.xaxis.set_minor_locator(minorLocator)
            #  ax.set_ylim( ymax=1)
            ax.set_ylim( bottom=1e-6, top=1, auto=True, emit=True)
            ax.set_title("{} {}".format(group_name, scenario["name"]))
            ax.set_ylabel("P_block")
            ax.set_xlabel("a")
            plot_id = plot_id +1
            #  plt.boxplot(x=tc_data_x, y=data_y)
            ax.legend()

    plt.show()


if __name__ == "__main__":
    main(sys.argv[1:])

