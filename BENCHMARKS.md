# Benchmark results

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
