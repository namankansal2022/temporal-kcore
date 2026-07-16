# Benchmark results

**Dataset:** `data/CollegeMsg.txt` — 1899 nodes, 59835 temporal edges, 13838 static edges, 58911 timestamps, ~194 day span. Binned to 86400s -> 192 snapshots.

| Algorithm | Result | Time (ms) |
|---|---|---:|
| temporal-degree k-core | max core = 197 | 0.7 |
| static k-core | max core = 20 | 1.6 |
| (k,h)-core, h=2 | max core = 14 | 1.3 |
| time-window k-core (mid 50%) | max core = 8 | 0.5 |
| span-core [0,4] | max core = 0 | 3.4 |
| (l,delta)-dense, l=3 delta=3 | 401 nodes | 13.1 |
| (mu,tau,eps)-stable, 3/2/0.3 | 767 cores | 13.1 |
| (n,k)-pseudocore, n=8 | max h = 57; 908 nodes @ k=10 | 7324.6 |
