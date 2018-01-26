#!/usr/bin/env python

"""
Visualizer of the topology.

Usage:
    topology_graph.py <TOPOLOGY_FILE> [--output-dir=DIR] [--view]
    topology_graph.py -h | --help

Arguments:
    TOPOLOGY_FILE   path to the file with topology

Options:
    -h --help                   show this help message and exit
    -v, --view                  show the output file with default viewier
    -d DIR, --output-dir=DIR    directory where the files are saved [default: data/results/]
"""
import json
from os import path
from docopt import docopt
import graphviz as gv


def apply_styles(graph, styles):
    graph.graph_attr.update(
        ('graph' in styles and styles['graph']) or {}
    )
    graph.node_attr.update(
        ('nodes' in styles and styles['nodes']) or {}
    )
    graph.edge_attr.update(
        ('edges' in styles and styles['edges']) or {}
    )
    return graph


def digraph(shape='box'):
    g = gv.Digraph()
    g.attr('graph', rank='same', nodesep='00.0', ordering='out')
    g.attr('node', shape=shape)
    return g


def main():
    args = docopt(__doc__, version='0.1')

    topology_file = args["<TOPOLOGY_FILE>"]
    topology = json.load(open(topology_file))

    #  g = gv.Digraph(format='pdf')
    g = gv.Digraph(format='pdf')

    styles = {
        'graph': {
            'fontsize': '12',
            'rankdir': 'LR',
            'ranksep': '1',
            'nodesep': '0.5',
        },
        'nodes': {
            'fontname': 'Helvetica',
            'shape': 'hexagon',
            'fontsize': '10',
            'style': 'filled',
            'fixedsize': 'true',
            'fillcolor': '#ccccff',
        },
        'edges': {
            'style': 'solid',
            'arrowhead': 'open',
            'fontname': 'Courier',
            'fontsize': '12',
        }
    }

    apply_styles(g, styles)

    source_nodes = gv.Digraph()
    source_nodes.attr('graph', rank='same')

    layers = {}

    tcs = topology["traffic_classes"]

    for group_name, data in topology["groups"].items():
        layer = layers.setdefault(data["layer"], digraph())
        layer.node(group_name, label="{}\nV={}".format(
            group_name, data["capacity"]))

    for _, layer in layers.items():
        g.subgraph(layer)

    for source_name, data in topology["sources"].items():
        source_nodes.node(source_name, shape='oval', width='1.0', height='0.5',
                          label="{0}\n{1}\nt={2}".format(source_name, data["type"],
                                                        tcs[str(data["traffic_class"])]["size"]))
        g.edge(source_name+":e", data["attached"]+":w", label="tc={}".format(
            data["traffic_class"]))

    g.subgraph(source_nodes)

    for group_name, data in topology["groups"].items():
        if "connected" in data:
            for connected_group in data["connected"]:
                g.edge(group_name + ":e", connected_group)

    output_dir = args["--output-dir"]
    output_file = path.splitext(path.basename(topology_file))[0]
    output_file = path.join(output_dir, "graph", output_file)
    print(output_file)
    g.render(output_file, cleanup=True,view=args["--view"])


if __name__ == "__main__":
    main()
