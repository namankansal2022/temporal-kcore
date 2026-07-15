#!/usr/bin/env python3
"""Run the temporal-graph analyses on a real SNAP-style dataset.

Usage:
    PYTHONPATH=build python python/analyze.py <edgelist> [bin_seconds] [l] [delta]

Basic statistics and the temporal-degree k-core use raw timestamps. The
(l, delta)-dense/bursting core needs meaningful snapshots, so we bin the raw
timestamps into fixed-width buckets (default 1 day) before running it.
"""
import sys
import time
import pytkcore


def load_edges(path):
    edges = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line[0] in "#%":
                continue
            p = line.split()
            if len(p) < 2:
                continue
            u, v = int(p[0]), int(p[1])
            t = int(p[2]) if len(p) >= 3 else 0
            edges.append((u, v, t))
    return edges


def main():
    if len(sys.argv) < 2:
        print("usage: analyze.py <edgelist> [bin_seconds] [l] [delta]")
        sys.exit(1)
    path = sys.argv[1]
    bin_seconds = int(sys.argv[2]) if len(sys.argv) > 2 else 86400
    l = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    delta = float(sys.argv[4]) if len(sys.argv) > 4 else 3.0

    g = pytkcore.load(path)
    s = pytkcore.statistics(g)
    print(f"dataset: {path}")
    print(f"  nodes                {s.num_nodes}")
    print(f"  temporal edges       {s.num_temporal_edges}")
    print(f"  static edges         {s.num_static_edges}")
    print(f"  distinct timestamps  {s.num_distinct_times}")
    print(f"  time span            {s.time_span} s  (~{s.time_span/86400:.1f} days)")
    print(f"  avg / max temp degree {s.avg_temporal_degree:.2f} / {s.max_temporal_degree}")

    t0 = time.time()
    core = pytkcore.core_numbers(g)
    dt = (time.time() - t0) * 1000
    max_core = max(core.values()) if core else 0
    n_top = sum(1 for c in core.values() if c == max_core)
    print(f"  temporal k-core: max core number = {max_core} "
          f"({n_top} nodes at that level)   [{dt:.1f} ms]")

    edges = load_edges(path)
    tmin = min(t for _, _, t in edges)
    gb = pytkcore.TemporalGraph()
    for u, v, t in edges:
        gb.add_edge(u, v, (t - tmin) // bin_seconds)
    gb.finalize()
    print(f"\nbinned to {bin_seconds}s buckets -> {gb.num_timestamps()} snapshots")

    for (ll, dd) in [(l, delta), (2, 2.0), (3, 2.0)]:
        t0 = time.time()
        dc = pytkcore.dense_core(gb, ll, dd)
        dt = (time.time() - t0) * 1000
        preview = sorted(dc)[:12]
        print(f"  (l={ll}, delta={dd}) dense core: {len(dc)} nodes {preview}"
              f"{' ...' if len(dc) > 12 else ''}   [{dt:.1f} ms]")


if __name__ == "__main__":
    main()
