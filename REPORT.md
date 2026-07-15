# Progress Report — Week 1

**Project:** Benchmarking Temporal k-Core Algorithms
**Focus this week:** set up the C++/Python toolchain, implement a temporal graph
loader with basic statistics, and implement a linear-time k-core using the
temporal degree.

---

## 1. Understanding of the material

### 1.1 Temporal networks (COMP324 fundamentals)

A temporal graph is `G = (V, E)` with `E = {(u, v, t)}`, where each edge carries
an integer timestamp. Unlike static analysis, edges are not always present:
aggregating over time can invent paths that never existed and overstate
connectivity. Key representations are the **sequence of temporal edges** (the
lossless one, which I use), the **sequence of snapshots** `G₁…G_T` (depends on a
quantization/time-resolution choice), and the **time-window / projected graph**
(static graph induced by edges in an interval, generally lossy). Choosing the
time resolution matters: coarse binning loses information, fine binning gives
sparse/empty steps. Concepts such as reachability and paths become
time-dependent and are generally harder than in static graphs.

### 1.2 Temporal cohesive structures and k-cores (survey talk)

Cohesive structures are vertex sets that "stick together" via many internal
edges; the **static k-core** is the maximal subgraph in which every vertex has
degree ≥ k, with a unique, nested (`C_{k+1} ⊆ C_k`) decomposition computable by a
single **peeling** pass (remove the current minimum-degree vertex, update
neighbours). This is exactly the linear-time algorithm I implemented.

The temporal case has **no single agreed definition** — the survey lists many
variants (`(k,h)`-core, span-core, historical / time-range k-core, `(k,∆)`-core,
etc.). They differ mainly in *what temporal evidence counts*: repeated
interaction, persistence over an interval, snapshot stability, or local temporal
edge support. A recurring algorithmic principle is **monotonicity**: stricter
parameters shrink the core, which is what makes peeling-style decompositions
possible. The practical message for this project is that the main open work is
not inventing more definitions but making the existing ones **comparable,
efficient, and usable** — i.e. benchmarking, which is our goal.

### 1.3 Why the two papers were shared

* **Batagelj & Zaveršnik (2003), *O(m) cores decomposition*** — the foundation.
  It is the classic linear-time peeling algorithm (bin-sort by degree, peel in
  nondecreasing order, O(1) repair per edge) and the natural baseline every
  temporal variant builds on. I implemented the fast k-core directly from it.
* **Oettershagen, Konstantinidis & Italiano (2025), *edge-based `(k,∆)`
  framework*** — the frontier and one of the target algorithms to benchmark.
  Its evaluation section (datasets, baselines, runtime/memory metrics, choice of
  ∆, misinformation use case) is effectively the template for the benchmarking
  study I am building toward.

Together they bracket the project's scope: the classical static case on one end,
the state-of-the-art temporal edge-based method on the other, with the many
survey variants in between.

---

## 2. What I implemented (this week's TODOs)

### TODO 1 — C++ environment, Python bindings, CMake ✅
CMake project producing four artifacts: a `tkcore` static library, a
`tkcore_benchmark` CLI, a `test_kcore` test binary, and a `pytkcore` Python
module via **pybind11**. The Python module is optional (skipped gracefully if
pybind11 is absent). Release build uses `-O3`.

### TODO 2 — Loader + basic statistics ✅
A fast buffered reader for SNAP-style `src dst timestamp` edge lists (comments
skipped; also accepts 2-column static lists). External node ids are interned to
a contiguous `[0, n)` range for cache-friendly flat arrays. Statistics reported:
number of nodes, temporal edges, distinct **static** edges `{u,v}`, distinct
timestamps, time span, and average / maximum temporal degree.

### TODO 3 — Linear-time k-core with temporal degree ✅
Batagelj–Zaveršnik bin-sort peeling in `O(|V| + |E|)`. The **temporal degree**
of a node is the number of temporal edges incident to it: I build per-node
incidence lists in which a neighbour reached by *m* parallel temporal edges
appears *m* times, so the multi-edge degree and the per-edge relaxation both
fall out naturally. This computes the k-core of the underlying **temporal
multigraph**.

---

## 3. Correctness / accuracy

I wrote a second, independent reference implementation (lazy min-priority-queue
peeling with explicit multiplicity) purely for validation, and check the two
against each other:

* **Worked example** and **multiplicity example**: pass.
* **Randomized fuzzing**: 2000/2000 random temporal multigraphs — the fast
  algorithm's core numbers match the reference exactly.
* **Benchmark `--validate`** on data: 0 mismatches.

This gives me a trustworthy "ground truth" to measure future algorithms'
accuracy against.

---

## 4. Benchmark results so far

Machine used for these numbers is the dev container; absolute times will differ
on other hardware, but the **scaling trend** is the point. Synthetic inputs,
`k-core (fast)` = the linear-time peeling step only.

| temporal edges | k-core time | peak RSS |
|---------------:|------------:|---------:|
| 100 K          | 3.6 ms      | ~5 MiB   |
| 500 K          | 33 ms       | ~30 MiB  |
| 2 M            | 187 ms      | ~150 MiB |
| 4 M            | 346 ms      | ~260 MiB |
| 8 M            | 767 ms      | ~450 MiB |

Doubling the edges roughly doubles the k-core time at the larger sizes,
consistent with the expected `O(|V| + |E|)` behaviour. On the small synthetic
sanity input (40 nodes, 292 temporal edges) the loader, statistics, and k-core
all complete in well under a millisecond and validation passes.

The metrics tracked (time, peak memory, node/edge/timestamp counts, accuracy vs.
reference) are exactly the axes the benchmarking study will need.

---

## 5. Design decisions & assumptions

* **Undirected incidence.** k-core is computed on the undirected multigraph;
  each temporal edge contributes to both endpoints' temporal degree. (SNAP files
  are often directed; direction is preserved in the stored edge list and can be
  used later.)
* **Self-loops** are kept as-is by the loader; the fuzz tests ignore them. Easy
  to add a filter if the supervisor prefers.
* **Temporal-degree k-core ≠ (k,h)-core.** The `(k,h)`-core counts *neighbours*
  meeting a multiplicity threshold, whereas here multiplicity is added into the
  degree directly. I've kept this explicit so later variants slot in cleanly.
* **Loader distinct-timestamp counting** currently sorts timestamps
  (`O(|E| log |E|)`), which slightly dominates load time on the largest inputs.
  The k-core itself stays linear; I can make the timestamp count linear if load
  time becomes a concern.

---

## 6. Next steps (proposed)

1. Run on **real SNAP data**, starting with CollegeMsg (~60 K temporal edges),
   then email-Eu-core-temporal, then scale toward the `sx-*` and
   `wiki-talk-temporal` millions-scale datasets.
2. Add the **static simple-graph k-core** as a second baseline (distinct from
   the multigraph version) so the benchmark has more than one reference point.
3. Begin implementing the first "real" temporal variant — most likely the
   `(k,h)`-core (H. Wu et al., 2015), since it reduces to static core
   decomposition on filtered graphs and is a clean next step.
4. Add CSV output to the benchmark so results can be plotted for the
   poster/summary.

**Repository:** all code, tests, build instructions, and this report are in the
GitHub repo.
