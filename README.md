# Temporal k-Core — Benchmarking Toolkit

A C++ library (with Python bindings) for loading temporal graphs, computing
basic statistics, and computing a family of **temporal k-core decompositions**,
plus a benchmark harness that runs them all on
[Stanford SNAP](https://snap.stanford.edu/data/index.html#temporal) datasets.
Every algorithm ships with an independent reference implementation and is
fuzz-tested against it.

## Algorithms implemented

| Algorithm | Idea | Source |
|---|---|---|
| temporal-degree k-core | k-core using multi-edge (temporal) degree | Batagelj & Zaveršnik 2003 |
| static k-core | k-core on the de-temporalised simple graph | Seidman 1983 |
| (k,h)-core | keep pairs interacting ≥ h times, then k-core | Wu et al. 2015 |
| time-window / historical k-core | k-core over edges in a time window | Yu 2021 / Yang 2023 |
| span-core | k-core of the intersection graph over a span | Galimberti et al. 2021 |
| (l,δ)-dense / bursting core | avg degree ≥ δ over ≥ l consecutive snapshots | Qin et al. 2022 |
| (µ,τ,ε)-stable core | temporal SCAN structural clustering | Qin et al. 2020 |
| (η,k)-pseudocore | n-th order temporal H-index | Oettershagen et al. 2023 |
| (θ,τ)-persistent k-core | polynomial persistence decomposition* | Li et al. 2018 |

\* polynomial per-vertex decomposition; the NP-hard *largest*-persistent-core
community search is out of scope (see `REPORT.md`).

## Layout
include/tkcore/   headers: temporal_graph, loader, statistics, kcore,
dense_core, stable_core, filtered_core (static/(k,h)/window),
span_core, pseudocore, persistent_core
src/              implementations + pybind11 bindings
apps/             tkcore_benchmark CLI
tests/            7 fuzz/correctness test suites (test_kcore, test_dense,
test_stable, test_filtered, test_span, test_pseudo, test_persistent)
python/           analyze.py, benchmark_all.py, example.py, generate_synthetic.py
data/             sample_small.tedges (synthetic smoke-test input)
BENCHMARKS.md     latest benchmark results on CollegeMsg

## Build

Requirements: a C++17 compiler, CMake ≥ 3.15, and `pip install pybind11`.

```bash
cmake -S . -B build -Dpybind11_DIR=$(python -m pybind11 --cmakedir)
cmake --build build -j
ctest --test-dir build          # runs all 7 test suites
```

## Run

```bash
# get a real dataset
curl -L -o data/CollegeMsg.txt.gz https://snap.stanford.edu/data/CollegeMsg.txt.gz
gunzip -f data/CollegeMsg.txt.gz

# full benchmark across all algorithms (writes BENCHMARKS.md)
PYTHONPATH=build python python/benchmark_all.py data/CollegeMsg.txt 86400

# single-dataset analysis (stats, k-core, daily-binned dense core)
PYTHONPATH=build python python/analyze.py data/CollegeMsg.txt
```

## Python API (module `pytkcore`)

`load`, `statistics`, `core_numbers`, `static_core_numbers`, `kh_core_numbers`,
`window_core_numbers`, `num_snapshots`, `span_core_numbers`, `dense_core`,
`stable_core_nodes`, `stable_communities`, `temporal_h_index`, `pseudocore`,
`persistence_values`, `persistent_core`.

## Notes on modelling

- Degree-based cores use raw timestamps; snapshot-based cores (dense, stable,
  span, persistent) treat each distinct timestamp as a snapshot, so on
  per-second data you bin time first (the Python scripts do this).
- Each algorithm is validated against an independent reference implementation
  via randomized fuzz testing.
