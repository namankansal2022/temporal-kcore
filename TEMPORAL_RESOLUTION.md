# Temporal Resolution Benchmark

This benchmark evaluates the effect of temporal aggregation on the four
snapshot-based algorithms implemented in this project.

**Dataset**
- CollegeMsg

**Temporal resolutions evaluated**
- 1 hour
- 6 hours
- 12 hours
- 1 day
- 7 days

**Metrics collected**
- Runtime
- Peak memory usage
- Number of generated snapshots

As expected, finer temporal resolutions produce significantly more snapshots,
which increases the computational cost of snapshot-based algorithms.


| Resolution | Snapshots | Algorithm | Result | Status | Time (ms) | Peak (MB) |
|---|---:|---|---|---|---:|---:|
| 1 hour | 3313 | span_core | max core = 0 | ok | 3 | 34 |
| 1 hour | 3313 | dense_core | 0 nodes | ok | 185 | 35 |
| 1 hour | 3313 | stable_core | 912 cores | ok | 18 | 41 |
| 1 hour | 3313 | persistent_core | 138 nodes | ok | 40 | 35 |
| 6 hours | 729 | span_core | max core = 0 | ok | 3 | 34 |
| 6 hours | 729 | dense_core | 0 nodes | ok | 50 | 34 |
| 6 hours | 729 | stable_core | 856 cores | ok | 15 | 39 |
| 6 hours | 729 | persistent_core | 564 nodes | ok | 15 | 35 |
| 12 hours | 380 | span_core | max core = 0 | ok | 3 | 34 |
| 12 hours | 380 | dense_core | 272 nodes | ok | 23 | 34 |
| 12 hours | 380 | stable_core | 820 cores | ok | 14 | 39 |
| 12 hours | 380 | persistent_core | 745 nodes | ok | 11 | 35 |
| 1 day | 192 | span_core | max core = 0 | ok | 3 | 34 |
| 1 day | 192 | dense_core | 401 nodes | ok | 12 | 34 |
| 1 day | 192 | stable_core | 767 cores | ok | 13 | 38 |
| 1 day | 192 | persistent_core | 879 nodes | ok | 8 | 35 |
| 7 days | 28 | span_core | max core = 0 | ok | 3 | 34 |
| 7 days | 28 | dense_core | 687 nodes | ok | 5 | 34 |
| 7 days | 28 | stable_core | 290 cores | ok | 10 | 36 |
| 7 days | 28 | persistent_core | 1112 nodes | ok | 5 | 34 |
