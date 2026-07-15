#!/usr/bin/env python3
"""Minimal example of using the pytkcore Python module.

Build the module first (see README), then run this from the build directory
(or add the build directory to PYTHONPATH) with a dataset path:

    PYTHONPATH=build python python/example.py data/sample_small.tedges
"""
import sys
import pytkcore


def main(path):
    g = pytkcore.load(path)
    print(g)

    s = pytkcore.statistics(g)
    print(s)
    print(f"  nodes={s.num_nodes} temporal_edges={s.num_temporal_edges} "
          f"static_edges={s.num_static_edges} timestamps={s.num_distinct_times}")
    print(f"  avg_temporal_degree={s.avg_temporal_degree:.3f} "
          f"max_temporal_degree={s.max_temporal_degree}")

    core = pytkcore.core_numbers(g)
    max_core = max(core.values()) if core else 0
    print(f"max core number = {max_core}")

    # show a few nodes with the highest core numbers
    top = sorted(core.items(), key=lambda kv: kv[1], reverse=True)[:10]
    print("top nodes by core number (node_id: core):")
    for node, c in top:
        print(f"  {node}: {c}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("usage: python example.py <edgelist>")
        sys.exit(1)
    main(sys.argv[1])
