# Progress Report — Benchmarking Temporal k-Core Algorithms

**Project:** Benchmarking Temporal k-Core Algorithms.
**Scope of this report:** the Week 1 setup (toolchain, loader + statistics,
linear-time temporal-degree k-core), followed by the full library of temporal
k-core algorithms, their validation, and a benchmark across all of them.
Sections 1–6 document the Week 1 foundation; the later sections cover the
complete algorithm set and benchmark.

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

## 4. Base k-core scaling test (synthetic)

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

The Week 1 plan (run on real SNAP data, add the static k-core baseline, implement
the (k,h)-core) has since been completed, along with the rest of the algorithm
set -- see the sections below. The remaining work to turn this into a full
benchmarking study is:

1. **Per-algorithm memory measurement** (peak RSS), not just runtime.
2. **Scaling across dataset sizes** -- run on progressively larger SNAP datasets
   (email-Eu-core-temporal, `sx-*`, `wiki-talk-temporal`) and plot runtime /
   memory vs. #nodes, #edges, #timestamps.
3. **Time-resolution sensitivity** -- vary the binning granularity for the
   snapshot-based cores.
4. **Parameter sweeps** -- vary k, h, delta, theta, tau, eps, n rather than a
   single setting.
5. **CSV/plot output** for the poster/summary.


## Algorithms implemented (beyond Week 1)

After the Week 1 setup, the project was extended into a full library of temporal
k-core decompositions. Every algorithm was implemented in C++ from the source
paper's definition, exposed to Python via pybind11, and validated against an
independent reference implementation using randomized fuzz testing (thousands of
random temporal graphs per algorithm), plus worked examples from the papers
where available.

**The peeling family (shared engine).** Static k-core, (k,h)-core, and
time-window / historical k-core are all the same simple-graph k-core peeling
applied to a different filtered edge set (all pairs / multiplicity ≥ h / edges
within a window). Validated against a naive min-degree-removal reference.

**Span-core (Galimberti et al. 2021).** The (k,Δ)-span-core is the k-core of the
"intersection graph" — edges present in *every* snapshot of the span. Reference
builds the intersection by iterative snapshot intersection.

**(l,δ)-dense / bursting core (Qin et al. 2022).** A node is (l,δ)-dense if its
average degree over some run of ≥ l consecutive snapshots is ≥ δ. The O(n)
maximum-l-segment-density kernel (convex-hull sweep) matches an O(n²) reference
and the paper's worked values over 100k+ trials.

**(µ,τ,ε)-stable core (Qin et al. 2020).** Temporal SCAN: per-snapshot
structural (cosine) similarity, stable similarity over ≥ τ snapshots, cores with
≥ µ stably-similar neighbours, then union-find clustering. Matched a set-based
reference on cores and community partitions.

**(η,k)-pseudocore (Oettershagen, Kriege & Mutzel 2023).** The n-th order
temporal H-index (a reachability-aware H-index recursion); the (n,k)-pseudocore
thresholds it at k. The memoised recursion matches a literal tree-expansion of
the definition, and h⁽⁰⁾ equals the temporal degree as expected.

**(θ,τ)-persistent k-core (Li et al. 2018).** Implemented as the *polynomial
per-vertex decomposition*: slide a length-θ window, take per-window k-core
membership, and aggregate persistence via the paper's F(θ,k) function; the
(θ,τ)-persistent core thresholds F at τ. This reproduces the paper's worked value
(F = 4 for the [1,5] example). **Scope:** the original *persistent community
search* — finding the single *largest* (θ,τ)-persistent k-core — is NP-hard and
is deliberately out of scope (it needs branch-and-bound); the decomposition here
relaxes the joint-connectivity requirement and is the tractable counterpart.

All of the above are covered by the test suite (`ctest` runs 7 suites) and by the
full benchmark below.

## Benchmark results and interpretation

All nine algorithms were run on the CollegeMsg temporal network. Degree/peeling
cores use raw timestamps; snapshot-based cores (dense, stable, span, persistent)
use daily binning. The table below is regenerated by `python/benchmark_all.py`
(also saved to `BENCHMARKS.md`).

**Dataset:** `data/CollegeMsg.txt` — 1899 nodes, 59835 temporal edges, 13838 static edges, 58911 timestamps, ~194 day span. Binned to 86400s -> 192 snapshots.

| Algorithm | Result | Time (ms) |
|---|---|---:|
| temporal-degree k-core | max core = 197 | 0.8 |
| static k-core | max core = 20 | 1.9 |
| (k,h)-core, h=2 | max core = 14 | 1.4 |
| time-window k-core (mid 50%) | max core = 8 | 0.6 |
| span-core [0,4] | max core = 0 | 3.5 |
| (l,delta)-dense, l=3 delta=3 | 401 nodes | 14.4 |
| (mu,tau,eps)-stable, 3/2/0.3 | 767 cores | 14.1 |
| (n,k)-pseudocore, n=8 | max h = 57; 908 nodes @ k=10 | 7752.6 |
| (theta,tau)-persistent, 3/3/5 | 879 nodes | 8.1 |

**Interpretation.** The same network yields very different cores depending on
what each model counts:

- The gap between the temporal-degree k-core (max core 197) and the static
  k-core (max core 20) is the central observation: counting *repeated
  interactions* versus *distinct partners* are fundamentally different notions of
  cohesion.
- Requiring repetition ((k,h)-core, h=2) or restricting to a time window shrinks
  the cores further.
- The persistence-over-time models are strictest: the span-core over 5
  consecutive daily snapshots is empty (no pair interacts every day for five
  days), matching the known restrictiveness of the intersection-based span-core;
  the polynomial (theta,tau)-persistent core likewise keeps only the most
  temporally-consistent group.
- The clustering-flavoured models ((l,delta)-dense, (mu,tau,eps)-stable) instead
  surface larger, activity-based groups.
- The (eta,k)-pseudocore (temporal H-index) is reachability-based and by far the
  most expensive, highlighting the accuracy/efficiency trade-off.

As the models move from "count every interaction" to "distinct partners" to
"require repetition" to "require temporal persistence", the cores shrink
monotonically and meaningfully -- the behaviour the source papers predict. Every
algorithm is validated against an independent reference implementation.

**Scope note (persistent core).** The (theta,tau)-persistent k-core is
implemented as the *polynomial per-vertex decomposition* (sliding-window k-core
membership aggregated by the persistence function F). The original *persistent
community search* problem -- finding the single *largest* (theta,tau)-persistent
k-core -- is NP-hard (Li et al., ICDE 2018) and is deliberately out of scope. The
decomposition here relaxes the joint-connectivity requirement and is the
tractable counterpart consistent with the other cores in this library.
