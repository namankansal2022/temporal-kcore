# Scaling benchmark

Real SNAP datasets, 1 subprocess per run, 300s timeout.

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
