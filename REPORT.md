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


## 7. Algorithms implemented (beyond Week 1)

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

## 8. Benchmark results and interpretation

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


## 9. Scaling across real SNAP datasets

All algorithms were run on real SNAP datasets from ~60K to ~1.4M edges, each in
its own subprocess with a 300 s timeout and peak-memory measurement (see
`SCALING.md` / `SCALING.csv`, produced by `python/scaling_benchmark.py`).

| Dataset | Edges | Algorithm | Result | Status | Time (ms) | Peak (MB) |
|---|---:|---|---|---|---:|---:|
| CollegeMsg.txt | 59835 | temporal_kcore | max core = 197 | ok | 1 | 25 |
| CollegeMsg.txt | 59835 | static_kcore | max core = 20 | ok | 1 | 25 |
| CollegeMsg.txt | 59835 | kh_core | max core = 14 | ok | 1 | 25 |
| CollegeMsg.txt | 59835 | window_core | max core = 8 | ok | 0 | 25 |
| CollegeMsg.txt | 59835 | span_core | max core = 0 | ok | 3 | 36 |
| CollegeMsg.txt | 59835 | dense_core | 401 nodes | ok | 11 | 36 |
| CollegeMsg.txt | 59835 | stable_core | 767 cores | ok | 12 | 40 |
| CollegeMsg.txt | 59835 | pseudocore | max h = 57 | ok | 7340 | 87 |
| CollegeMsg.txt | 59835 | persistent_core | 879 nodes | ok | 8 | 36 |
| email-Eu-core-temporal.txt | 332334 | temporal_kcore | max core = 4992 | ok | 2 | 43 |
| email-Eu-core-temporal.txt | 332334 | static_kcore | max core = 34 | ok | 2 | 44 |
| email-Eu-core-temporal.txt | 332334 | kh_core | max core = 26 | ok | 2 | 44 |
| email-Eu-core-temporal.txt | 332334 | window_core | max core = 28 | ok | 2 | 44 |
| email-Eu-core-temporal.txt | 332334 | span_core | max core = 1 | ok | 10 | 103 |
| email-Eu-core-temporal.txt | 332334 | dense_core | 428 nodes | ok | 44 | 108 |
| email-Eu-core-temporal.txt | 332334 | stable_core | 754 cores | ok | 83 | 125 |
| email-Eu-core-temporal.txt | - | pseudocore | - | timeout | 300000 | - |
| email-Eu-core-temporal.txt | 332334 | persistent_core | 676 nodes | ok | 42 | 111 |
| sx-mathoverflow.txt | 506550 | temporal_kcore | max core = 3888 | ok | 7 | 58 |
| sx-mathoverflow.txt | 506550 | static_kcore | max core = 78 | ok | 16 | 66 |
| sx-mathoverflow.txt | 506550 | kh_core | max core = 48 | ok | 14 | 65 |
| sx-mathoverflow.txt | 506550 | window_core | max core = 47 | ok | 11 | 63 |
| sx-mathoverflow.txt | 506550 | span_core | max core = 0 | ok | 36 | 166 |
| sx-mathoverflow.txt | 506550 | dense_core | 1652 nodes | ok | 1565 | 168 |
| sx-mathoverflow.txt | 506550 | stable_core | 3892 cores | ok | 209 | 218 |
| sx-mathoverflow.txt | - | pseudocore | - | timeout | 300000 | - |
| sx-mathoverflow.txt | 506550 | persistent_core | 3377 nodes | ok | 390 | 172 |
| sx-askubuntu.txt | 964437 | temporal_kcore | max core = 3128 | ok | 33 | 112 |
| sx-askubuntu.txt | 964437 | static_kcore | max core = 48 | ok | 70 | 130 |
| sx-askubuntu.txt | 964437 | kh_core | max core = 29 | ok | 60 | 127 |
| sx-askubuntu.txt | 964437 | window_core | max core = 40 | ok | 52 | 122 |
| sx-askubuntu.txt | 964437 | span_core | max core = 0 | ok | 133 | 335 |
| sx-askubuntu.txt | 964437 | dense_core | 802 nodes | ok | 3760 | 333 |
| sx-askubuntu.txt | 964437 | stable_core | 6366 cores | ok | 468 | 433 |
| sx-askubuntu.txt | - | pseudocore | - | timeout | 300000 | - |
| sx-askubuntu.txt | 964437 | persistent_core | 10167 nodes | ok | 1864 | 332 |
| sx-superuser.txt | 1443339 | temporal_kcore | max core = 7252 | ok | 45 | 157 |
| sx-superuser.txt | 1443339 | static_kcore | max core = 61 | ok | 112 | 187 |
| sx-superuser.txt | 1443339 | kh_core | max core = 36 | ok | 94 | 184 |
| sx-superuser.txt | 1443339 | window_core | max core = 50 | ok | 76 | 176 |
| sx-superuser.txt | 1443339 | span_core | max core = 0 | ok | 224 | 492 |
| sx-superuser.txt | 1443339 | dense_core | 1824 nodes | ok | 8192 | 491 |
| sx-superuser.txt | 1443339 | stable_core | 10593 cores | ok | 880 | 649 |
| sx-superuser.txt | - | pseudocore | - | timeout | 300000 | - |
| sx-superuser.txt | 1443339 | persistent_core | 17769 nodes | ok | 2917 | 473 |

**Findings.**

- *Eight of nine algorithms successfully complete on all datasets evaluated in this section (up to 1.4M edges).* The peeling family
  (temporal-degree, static, (k,h), time-window) finishes in tens of milliseconds
  at 1.4M edges with linear memory; span-core, (l,delta)-dense,
  (mu,tau,eps)-stable and (theta,tau)-persistent also complete at every dataset
  evaluated here (dense is the slowest, ~8 s at 1.4M; stable is the heaviest,
  ~650 MB).

- *The recursive (eta,k)-pseudocore does not scale.* It completes on CollegeMsg
  (~7 s) but times out (>300 s) on every dataset from 332K edges upward.
  This is expected for the memoised recursion and is exactly why the source work
  uses a streaming algorithm.

- *Temporal-degree vs static coreness diverges sharply at scale.* The
  temporal-degree max core reaches into the thousands while the static k-core
  stays in the tens, illustrating repeated interactions rather than cohesion.

- *Span-core is consistently 0–1* across all five datasets, confirming that the
  intersection-based span-core is very restrictive on real interaction data.


## 10. Scaling to tens of millions of temporal edges

To evaluate scalability beyond the million-edge regime, the benchmark was
extended to two substantially larger SNAP datasets:

- **wiki-talk-temporal** (~7.8 million temporal edges)
- **sx-stackoverflow** (~63.5 million temporal edges)

The recursive (η,k)-pseudocore was intentionally excluded because Section 9 had
already established that it consistently exceeds the 300-second timeout beyond
approximately 332K edges.

| Dataset | Edges | Algorithm | Result | Status | Time (ms) | Peak (MB) |
|---|---:|---|---|---|---:|---:|
| wiki-talk-temporal.txt | 7833140 | temporal_kcore | max core = 62900 | ok | 201 | 701 |
| wiki-talk-temporal.txt | 7833140 | static_kcore | max core = 124 | ok | 446 | 814 |
| wiki-talk-temporal.txt | 7833140 | kh_core | max core = 77 | ok | 374 | 784 |
| wiki-talk-temporal.txt | 7833140 | window_core | max core = 82 | ok | 202 | 717 |
| wiki-talk-temporal.txt | 7833140 | span_core | max core = 0 | ok | 1255 | 2000 |
| wiki-talk-temporal.txt | 7833140 | dense_core | 9352 nodes | ok | 47871 | 2115 |
| wiki-talk-temporal.txt | 7833140 | stable_core | 25767 cores | ok | 4690 | 2250 |
| wiki-talk-temporal.txt | 7833140 | persistent_core | 55915 nodes | ok | 15478 | 2259 |
| sx-stackoverflow.txt | 63497050 | temporal_kcore | max core = 59838 | ok | 1718 | 2377 |
| sx-stackoverflow.txt | 63497050 | static_kcore | max core = 198 | ok | 5715 | 2284 |
| sx-stackoverflow.txt | 63497050 | kh_core | max core = 100 | ok | 4795 | 2414 |
| sx-stackoverflow.txt | 63497050 | window_core | max core = 154 | ok | 3748 | 2457 |
| sx-stackoverflow.txt | 63497050 | span_core | max core = 0 | ok | 31957 | 2155 |
| sx-stackoverflow.txt | - | dense_core | - | timeout | 300000 | - |
| sx-stackoverflow.txt | - | stable_core | - | timeout | 300000 | - |
| sx-stackoverflow.txt | 63497050 | persistent_core | 883778 nodes | ok | 62850 | 2272 |

### Interpretation

The larger-scale experiments reinforce the conclusions from Section 9 while
showing where the practical scalability limits begin to appear.

- **The peeling family scales exceptionally well.** Temporal-degree k-core,
  static k-core, (k,h)-core and time-window k-core all continue to complete on
  the 63.5-million-edge StackOverflow network in only a few seconds, confirming
  their near-linear behaviour in practice.

- **Span-core remains tractable.** Although substantially slower than the
  peeling algorithms (about 32 seconds on StackOverflow), it still completes
  within the timeout while consistently producing a maximum span-core of 0 on
  these communication datasets, illustrating the restrictive nature of the
  intersection-based formulation.

- **Dense-core reaches its practical scalability limit.** It completes on the
  7.8-million-edge WikiTalk dataset but exceeds the 300-second timeout on the
  63.5-million-edge StackOverflow dataset.

- **Stable-core shows similar behaviour.** It remains practical on WikiTalk but
  also exceeds the timeout on StackOverflow, indicating that its structural
  similarity computations become increasingly expensive at very large scales.

- **Persistent-core continues to scale.** Although slower than the peeling
  algorithms, it successfully completes even on the largest dataset (approximately
  63 seconds), demonstrating substantially better scalability than dense-core
  and stable-core.

- **Temporal-degree and static coreness diverge even more strongly at scale.**
  On StackOverflow the temporal-degree maximum core reaches 59,838 whereas the
  static maximum core is only 198. This confirms that repeated temporal
  interactions dominate temporal-degree measures while static cores capture only
  the diversity of neighbours.

### Overall scalability conclusion

Across all experiments, the benchmark now spans datasets from roughly **60
thousand** temporal edges to more than **63 million** temporal edges.

The empirical results show three distinct scalability regimes:

1. **Highly scalable algorithms:** temporal-degree k-core, static k-core,
   (k,h)-core, time-window k-core, span-core and persistent-core successfully
   process datasets containing tens of millions of temporal edges.

2. **Moderately scalable algorithms:** (l,δ)-dense and (µ,τ,ε)-stable remain
   practical up to several million temporal edges but exceed the five-minute
   timeout on the largest StackOverflow dataset.

3. **Least scalable algorithm:** the recursive (η,k)-pseudocore exceeds the
   timeout once datasets reach a few hundred thousand temporal edges, confirming
   that a streaming implementation would be required for large-scale networks.

These experiments complete the scalability evaluation requested for the project
and provide a comprehensive comparison of runtime and memory behaviour across
multiple temporal k-core formulations.

