# Temporal k-Core — Benchmarking Toolkit (Week 1)

A C++ library (with Python bindings) for loading temporal graphs, computing
basic statistics, and computing **temporal-degree k-core** numbers in linear
time. This is the foundation for benchmarking temporal k-core algorithms on
[Stanford SNAP](https://snap.stanford.edu/data/index.html#temporal) temporal
datasets.

## What's implemented (this week's TODOs)

1. **C++ environment + Python bindings + CMake** — a `tkcore` static library,
   a `tkcore_benchmark` CLI, a test binary, and a `pytkcore` Python module
   built with pybind11, all driven by CMake.
2. **Temporal graph loader + basic statistics** — reads SNAP-style
   `src dst timestamp` edge lists and reports number of nodes, temporal edges,
   distinct static edges, distinct timestamps, time span, and temporal-degree
   summary.
3. **Linear-time k-core with temporal degree** — Batagelj–Zaveršnik bin-sort
   peeling, `O(|V| + |E|)`, where the degree of a node is the number of
   temporal edges incident to it (parallel edges counted with multiplicity).
   Validated against an independent reference implementation.

## Layout

```
include/tkcore/   public headers (temporal_graph, loader, statistics, kcore)
src/              implementations + pybind11 bindings
apps/             tkcore_benchmark CLI
tests/            correctness + randomized fuzz tests
python/           example.py, generate_synthetic.py
data/             sample_small.tedges (synthetic smoke-test input)
```

## Build

Requirements: a C++17 compiler, CMake ≥ 3.15, and (for the Python module)
`pip install pybind11`.

```bash
cmake -S . -B build -Dpybind11_DIR=$(python -m pybind11 --cmakedir)
cmake --build build -j
```

The Python module is optional: if pybind11 is not found, the C++ library, CLI,
and tests still build.

## Run

```bash
# tests (unit + fuzz: fast algorithm vs. reference)
./build/test_kcore
ctest --test-dir build

# benchmark on a dataset, with correctness validation
./build/tkcore_benchmark data/sample_small.tedges --validate

# Python
PYTHONPATH=build python python/example.py data/sample_small.tedges
```

## Getting real data (SNAP)

Download any temporal edge list from SNAP and unzip it, e.g. CollegeMsg
(~1.9K nodes / ~60K temporal edges) as a small starting point, scaling up to
`sx-*` and `wiki-talk-temporal` (millions). Files are already in the expected
`src dst timestamp` format:

```bash
# example
wget https://snap.stanford.edu/data/CollegeMsg.txt.gz
gunzip CollegeMsg.txt.gz
./build/tkcore_benchmark CollegeMsg.txt --validate
```

If you don't have data yet, generate a synthetic file of any size:

```bash
python python/generate_synthetic.py --nodes 100000 --edges 2000000 \
    --out data/synth_2M.tedges
```

## Notes on the degree model

The core number here is with respect to the **temporal (multi-edge) degree**:
a node joined to a neighbour by three temporal edges gets +3 to its degree.
This is deliberately distinct from the static simple-graph k-core and from the
`(k,h)`-core, and is the natural first temporal baseline. Later variants
(`(k,h)`, span-core, `(k,∆)`, …) will be added as additional algorithms to
benchmark against this one.
