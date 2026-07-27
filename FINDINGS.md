# Research Summary — Benchmarking Temporal k-Core Algorithms

**Naman Kansal · Supervisor: Dr Lutz Oettershagen · COMP298**

## Overview
Nine temporal k-core decomposition algorithms — classical and recent — were
implemented in C++, validated against independent reference implementations
(randomized fuzz testing, 7/7 test suites passing), and benchmarked on real
Stanford SNAP temporal networks for runtime, memory, scalability, and temporal
resolution. Figures are in `figures/`; the poster is `poster.html`.

## Key findings

1. **Most algorithms scale to millions of edges.** Eight of nine complete in
   seconds with near-linear memory up to tens of millions of temporal edges
   (up to ~63M). The recursive (η,k)-pseudocore is the exception: it times out
   beyond ~300K edges and would require a streaming implementation to scale.
   *(figures/scaling_runtime.png, figures/scaling_memory.png)*

2. **The choice of definition dominates the result.** Temporal-degree coreness
   (which counts repeated interactions) diverges enormously from static coreness
   (distinct partners) — e.g. 62,900 vs. 124 on wiki-talk-temporal — driven by
   edge multiplicity from hyper-active accounts. This is the project's central
   observation: how you define "degree" changes the structural insight entirely.
   *(figures/coreness_comparison.png)*

3. **Temporal resolution changes both cost and outcome.** As binning moves from
   coarse to fine, the number of snapshots and the compute cost rise, and the
   cores themselves shift — the (l,δ)-dense core grows with coarser bins while
   the (µ,τ,ε)-stable core shrinks. Span-core is empty across all settings,
   confirming the intersection-based model is highly restrictive on real data.
   *(figures/resolution_result.png, figures/resolution_runtime.png)*

## Scope and limitations
- The (θ,τ)-persistent k-core is implemented as the polynomial decomposition;
  the NP-hard maximum-persistent community search is out of scope.
- The temporal-resolution sweep uses a single dataset (CollegeMsg); repeating it
  on additional datasets would strengthen the resolution findings.
