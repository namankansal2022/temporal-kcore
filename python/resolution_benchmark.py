#!/usr/bin/env python3
"""Benchmark snapshot algorithms across different temporal resolutions.

Example:

PYTHONPATH=build python python/resolution_benchmark.py \
    --dataset data/CollegeMsg.txt \
    --timeout 300 \
    --out TEMPORAL_RESOLUTION
"""

import argparse, json, os, subprocess, sys, time, resource

ALGOS = [
    "span_core",
    "dense_core",
    "stable_core",
    "persistent_core",
]

BINS = [
    ("1 hour",3600),
    ("6 hours",21600),
    ("12 hours",43200),
    ("1 day",86400),
    ("7 days",604800),
]

def peak_mb():
    m=resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    return m/(1024*1024) if sys.platform=="darwin" else m/1024.0

def run_one(gb,S,algo):
    mx=lambda d:max(d.values()) if d else 0

    if algo=="span_core":
        return "max core = %d"%mx(
            pytkcore.span_core_numbers(gb,0,min(4,S-1))
        )

    if algo=="dense_core":
        return "%d nodes"%len(
            pytkcore.dense_core(gb,3,3.0)
        )

    if algo=="stable_core":
        return "%d cores"%len(
            pytkcore.stable_core_nodes(gb,3,2,0.3)
        )

    if algo=="persistent_core":
        return "%d nodes"%len(
            pytkcore.persistent_core(gb,3,3,5)
        )

    raise ValueError(algo)


def worker(path,algo,binsec):

    global pytkcore
    import pytkcore

    edges=[]

    for line in open(path):

        line=line.strip()

        if not line or line[0] in "#%":
            continue

        p=line.split()

        if len(p)<2:
            continue

        edges.append((
            int(p[0]),
            int(p[1]),
            int(p[2]) if len(p)>=3 else 0
        ))

    tmin=min(t for _,_,t in edges)

    gb=pytkcore.TemporalGraph()

    for u,v,t in edges:
        gb.add_edge(
            u,
            v,
            (t-tmin)//binsec
        )

    gb.finalize()

    S=pytkcore.num_snapshots(gb)

    t0=time.perf_counter()

    res=run_one(gb,S,algo)

    dt=(time.perf_counter()-t0)*1000

    print(json.dumps({
        "snapshots":S,
        "time_ms":dt,
        "peak_mb":peak_mb(),
        "result":res
    }))


def main():

    ap=argparse.ArgumentParser()

    ap.add_argument("--dataset",required=True)
    ap.add_argument("--timeout",type=int,default=300)
    ap.add_argument("--out",default="TEMPORAL_RESOLUTION")
    ap.add_argument("--worker",nargs=3)

    a=ap.parse_args()

    if a.worker:
        worker(a.worker[0],a.worker[1],int(a.worker[2]))
        return

    rows=[]

    for label,binsec in BINS:

        print(
            "\nResolution: %s (%d seconds)"
            %(label,binsec),
            file=sys.stderr
        )

        for algo in ALGOS:

            print(
                "  %-18s ... "%algo,
                end="",
                file=sys.stderr,
                flush=True
            )

            cmd=[
                sys.executable,
                __file__,
                "--dataset",
                a.dataset,
                "--worker",
                a.dataset,
                algo,
                str(binsec)
            ]

            try:

                out=subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    timeout=a.timeout,
                    env={**os.environ}
                )

                if out.returncode==0:

                    d=json.loads(
                        out.stdout.strip().splitlines()[-1]
                    )

                    rows.append((
                        label,
                        binsec,
                        d["snapshots"],
                        algo,
                        d["result"],
                        "ok",
                        d["time_ms"],
                        d["peak_mb"]
                    ))

                    print(
                        "ok %.0f ms %.0f MB (%d snapshots)"
                        %(
                            d["time_ms"],
                            d["peak_mb"],
                            d["snapshots"]
                        ),
                        file=sys.stderr
                    )

                else:

                    print("\n----- SUBPROCESS STDERR -----", file=sys.stderr)
                    print(out.stderr, file=sys.stderr)
                    print("----- END STDERR -----\n", file=sys.stderr)

                    rows.append((
                        label,
                        binsec,
                        0,
                        algo,
                        out.stderr.strip(),
                        "error",
                        0,
                        0
                    ))

                    print("ERROR",file=sys.stderr)

            except subprocess.TimeoutExpired:

                rows.append((
                    label,
                    binsec,
                    0,
                    algo,
                    "",
                    "timeout",
                    a.timeout*1000,
                    0
                ))

                print(
                    "TIMEOUT",
                    file=sys.stderr
                )

    import csv

    with open(a.out+".csv","w",newline="") as f:

        w=csv.writer(f)

        w.writerow([
            "resolution",
            "bin_seconds",
            "snapshots",
            "algorithm",
            "result",
            "status",
            "time_ms",
            "peak_mb"
        ])

        for r in rows:
            w.writerow(r)

    with open(a.out+".md","w") as f:

        f.write("# Temporal Resolution Benchmark\n\n")

        f.write(
            "| Resolution | Snapshots | Algorithm | Result | Status | Time (ms) | Peak (MB) |\n"
        )

        f.write(
            "|---|---:|---|---|---|---:|---:|\n"
        )

        for res,sec,snap,algo,result,status,tm,mb in rows:

            f.write(
                "| %s | %s | %s | %s | %s | %s | %s |\n"
                %(
                    res,
                    snap if snap else "-",
                    algo,
                    result or "-",
                    status,
                    "%.0f"%tm if status in ("ok","timeout") else "-",
                    "%.0f"%mb if status=="ok" else "-"
                )
            )

    print(
        "\nwrote %s.csv and %s.md (%d rows)"
        %(a.out,a.out,len(rows)),
        file=sys.stderr
    )

if __name__=="__main__":
    main()

