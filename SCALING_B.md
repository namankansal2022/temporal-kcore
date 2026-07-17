# Scaling benchmark

Real SNAP datasets, 1 subprocess per run, 300s timeout.

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
