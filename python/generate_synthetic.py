#!/usr/bin/env python3
"""Generate a synthetic temporal edge list in SNAP format ('src dst timestamp').

Useful for smoke-testing the pipeline before downloading real SNAP data, and
for producing inputs of a chosen size when scaling up the benchmarks.

Example:
    python generate_synthetic.py --nodes 5000 --edges 200000 --out ../data/synth_5k.tedges
"""
import argparse
import random


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--nodes", type=int, default=1000)
    ap.add_argument("--edges", type=int, default=20000)
    ap.add_argument("--tmax", type=int, default=100000, help="max timestamp")
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--out", type=str, required=True)
    args = ap.parse_args()

    rng = random.Random(args.seed)
    with open(args.out, "w") as f:
        f.write("# synthetic temporal edge list: src dst timestamp\n")
        for _ in range(args.edges):
            u = rng.randrange(args.nodes)
            v = rng.randrange(args.nodes)
            if u == v:
                continue
            t = rng.randrange(args.tmax)
            f.write(f"{u} {v} {t}\n")
    print(f"wrote {args.out}: up to {args.edges} temporal edges over {args.nodes} nodes")


if __name__ == "__main__":
    main()
