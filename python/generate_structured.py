#!/usr/bin/env python3
"""Generate a *structured* synthetic temporal edge list ('src dst timestamp').

Unlike generate_synthetic.py (uniform Erdos-Renyi, which makes every core
definition agree trivially), this produces the heterogeneity found in real
communication networks, so the different temporal-core definitions actually
diverge:

  * skewed activity  - a few hub accounts generate many interactions;
  * repeated contact - some pairs interact many times, most only once
                       (this is what separates temporal-degree from static);
  * communities      - edges are denser within blocks than across them;
  * bursts           - activity is clustered in time, not uniform.

Purpose: a controllable stand-in so structural_analysis.py can be validated
end-to-end without downloading SNAP data. Real results should use CollegeMsg.

Example:
    python generate_structured.py --nodes 1900 --edges 60000 \
        --out ../data/synth_struct.tedges
"""
import argparse, random, math


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--nodes", type=int, default=1900)
    ap.add_argument("--edges", type=int, default=60000)
    ap.add_argument("--tmax", type=int, default=16 * 7 * 24 * 3600)  # ~16 weeks
    ap.add_argument("--communities", type=int, default=12)
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--out", type=str, required=True)
    args = ap.parse_args()
    rng = random.Random(args.seed)

    N, C = args.nodes, args.communities
    comm = [rng.randrange(C) for _ in range(N)]           # node -> community
    members = [[u for u in range(N) if comm[u] == c] for c in range(C)]

    # activity weight ~ power law: a few very active nodes, a long tail.
    weight = [1.0 / (1.0 + rng.expovariate(1.0)) ** 2 for _ in range(N)]
    for u in rng.sample(range(N), max(1, N // 60)):        # a handful of hubs
        weight[u] *= 40.0
    tot = sum(weight)
    weight = [w / tot for w in weight]
    cumw, acc = [], 0.0
    for w in weight:
        acc += w
        cumw.append(acc)

    def pick():
        r = rng.random()
        lo, hi = 0, N - 1
        while lo < hi:
            mid = (lo + hi) // 2
            if cumw[mid] < r:
                lo = mid + 1
            else:
                hi = mid
        return lo

    # A pool of "friendships" that recur; the rest of the mass is one-off edges.
    friends = []
    for _ in range(args.edges // 4):
        u = pick()
        pool = members[comm[u]] if rng.random() < 0.85 else range(N)
        v = rng.choice(pool)
        if u != v:
            friends.append((min(u, v), max(u, v)))
    friends = list(set(friends))

    def a_time():
        # bursty: pick a random "day", then a moment within it
        day = rng.randrange(max(1, args.tmax // 86400))
        return day * 86400 + rng.randrange(86400)

    lines = []
    for _ in range(args.edges):
        if friends and rng.random() < 0.55:                # recurring contact
            u, v = rng.choice(friends)
        else:                                              # one-off contact
            u = pick()
            pool = members[comm[u]] if rng.random() < 0.8 else range(N)
            v = rng.choice(pool)
            if u == v:
                continue
        lines.append((u, v, a_time()))

    lines.sort(key=lambda e: e[2])
    with open(args.out, "w") as f:
        f.write("# structured synthetic temporal edge list: src dst timestamp\n")
        for u, v, t in lines:
            f.write(f"{u} {v} {t}\n")
    print(f"wrote {args.out}: {len(lines)} temporal edges over {N} nodes, "
          f"{C} communities, {len(friends)} recurring pairs")


if __name__ == "__main__":
    main()
