#!/usr/bin/env python3
"""Generate figures from the benchmark CSVs for the poster/summary.

Reads SCALING.csv, SCALING_B.csv, TEMPORAL_RESOLUTION.csv (whichever exist) and
writes PNGs into figures/. Pure visualisation -- no new experiments.
"""
import csv, os, re
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

FIGDIR = "figures"
NICE = {
    "temporal_kcore":"temporal-degree k-core", "static_kcore":"static k-core",
    "kh_core":"(k,h)-core", "window_core":"time-window k-core",
    "span_core":"span-core", "dense_core":"(l,d)-dense", "stable_core":"(m,t,e)-stable",
    "pseudocore":"(eta,k)-pseudocore", "persistent_core":"(theta,tau)-persistent",
}

def load(path):
    if not os.path.exists(path): return []
    with open(path) as f: return list(csv.DictReader(f))

def num(s):
    m = re.search(r"-?\d+", s or ""); return int(m.group()) if m else None

def main():
    os.makedirs(FIGDIR, exist_ok=True)
    scaling = load("SCALING.csv") + load("SCALING_B.csv")
    res = load("TEMPORAL_RESOLUTION.csv")

    if scaling:
        algos = [a for a in NICE if any(r["algorithm"]==a for r in scaling)]
        plt.figure(figsize=(8,5))
        for a in algos:
            pts = sorted((int(r["edges"]), float(r["time_ms"]))
                         for r in scaling if r["algorithm"]==a and r["status"]=="ok" and int(r["edges"])>0)
            if pts:
                xs, ys = zip(*pts); plt.plot(xs, ys, marker="o", ms=4, label=NICE[a])
        plt.xscale("log"); plt.yscale("log")
        plt.xlabel("temporal edges"); plt.ylabel("runtime (ms)")
        plt.title("Scalability - runtime vs. number of temporal edges")
        plt.legend(fontsize=7, ncol=2); plt.grid(True, which="both", alpha=0.3)
        plt.tight_layout(); plt.savefig(f"{FIGDIR}/scaling_runtime.png", dpi=150); plt.close()

        plt.figure(figsize=(8,5))
        for a in algos:
            pts = sorted((int(r["edges"]), float(r["peak_mb"]))
                         for r in scaling if r["algorithm"]==a and r["status"]=="ok" and int(r["edges"])>0)
            if pts:
                xs, ys = zip(*pts); plt.plot(xs, ys, marker="s", ms=4, label=NICE[a])
        plt.xscale("log")
        plt.xlabel("temporal edges"); plt.ylabel("peak memory (MB)")
        plt.title("Scalability - peak memory vs. number of temporal edges")
        plt.legend(fontsize=7, ncol=2); plt.grid(True, which="both", alpha=0.3)
        plt.tight_layout(); plt.savefig(f"{FIGDIR}/scaling_memory.png", dpi=150); plt.close()

        ds_order = []
        for r in scaling:
            if r["dataset"] not in ds_order and int(r.get("edges") or 0) > 0:
                ds_order.append(r["dataset"])
        def core_of(dset, algo):
            for r in scaling:
                if r["dataset"]==dset and r["algorithm"]==algo and r["status"]=="ok":
                    return num(r["result"])
            return None
        td = [core_of(d,"temporal_kcore") for d in ds_order]
        st = [core_of(d,"static_kcore") for d in ds_order]
        keep = [i for i in range(len(ds_order)) if td[i] and st[i]]
        if keep:
            labels = [ds_order[i].replace(".txt","") for i in keep]
            x = range(len(keep)); w=0.4
            plt.figure(figsize=(9,5))
            plt.bar([i-w/2 for i in x], [td[i] for i in keep], w, label="temporal-degree k-core")
            plt.bar([i+w/2 for i in x], [st[i] for i in keep], w, label="static k-core")
            plt.yscale("log"); plt.xticks(list(x), labels, rotation=30, ha="right", fontsize=8)
            plt.ylabel("max core number (log)")
            plt.title("How the degree definition changes coreness")
            plt.legend(); plt.grid(True, axis="y", which="both", alpha=0.3)
            plt.tight_layout(); plt.savefig(f"{FIGDIR}/coreness_comparison.png", dpi=150); plt.close()

    if res:
        algos = [a for a in NICE if any(r["algorithm"]==a for r in res)]
        plt.figure(figsize=(8,5))
        for a in algos:
            pts = sorted((int(r["snapshots"]), float(r["time_ms"]))
                         for r in res if r["algorithm"]==a and r["status"]=="ok")
            if pts:
                xs, ys = zip(*pts); plt.plot(xs, ys, marker="o", ms=5, label=NICE[a])
        plt.xscale("log")
        plt.xlabel("number of snapshots (finer resolution ->)"); plt.ylabel("runtime (ms)")
        plt.title("Temporal resolution - runtime vs. snapshot count")
        plt.legend(fontsize=8); plt.grid(True, which="both", alpha=0.3)
        plt.tight_layout(); plt.savefig(f"{FIGDIR}/resolution_runtime.png", dpi=150); plt.close()

        plt.figure(figsize=(8,5))
        for a in algos:
            pts = sorted((int(r["snapshots"]), num(r["result"]))
                         for r in res if r["algorithm"]==a and r["status"]=="ok" and num(r["result"]) is not None)
            if pts:
                xs, ys = zip(*pts); plt.plot(xs, ys, marker="o", ms=5, label=NICE[a])
        plt.xscale("log")
        plt.xlabel("number of snapshots (finer resolution ->)"); plt.ylabel("core size / count")
        plt.title("Temporal resolution - result size vs. snapshot count")
        plt.legend(fontsize=8); plt.grid(True, which="both", alpha=0.3)
        plt.tight_layout(); plt.savefig(f"{FIGDIR}/resolution_result.png", dpi=150); plt.close()

    print("wrote figures:", ", ".join(sorted(os.listdir(FIGDIR))))

if __name__ == "__main__":
    main()
