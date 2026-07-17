#!/usr/bin/env python3
"""Scaling benchmark across real SNAP datasets.

Each (dataset x algorithm) runs in its own subprocess for a clean peak-memory
reading and a hard timeout (slow/huge runs are marked 'timeout' and skipped).
Writes a CSV and a Markdown table.

  PYTHONPATH=build python python/scaling_benchmark.py --datasets d1 d2 --timeout 300 --out SCALING
"""
import argparse, json, os, subprocess, sys, time, resource

ALGOS = ["temporal_kcore","static_kcore","kh_core","window_core",
         "span_core","dense_core","stable_core","persistent_core"]
SNAPSHOT = {"span_core","dense_core","stable_core","persistent_core"}

def peak_mb():
    m = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    return m/(1024*1024) if sys.platform=="darwin" else m/1024.0

def run_one(g, gb, S, q1, q3, algo):
    mx = lambda d: max(d.values()) if d else 0
    if algo=="temporal_kcore": return "max core = %d"%mx(pytkcore.core_numbers(g))
    if algo=="static_kcore":   return "max core = %d"%mx(pytkcore.static_core_numbers(g))
    if algo=="kh_core":        return "max core = %d"%mx(pytkcore.kh_core_numbers(g,2))
    if algo=="window_core":    return "max core = %d"%mx(pytkcore.window_core_numbers(g,q1,q3))
    if algo=="span_core":      return "max core = %d"%mx(pytkcore.span_core_numbers(gb,0,min(4,S-1)))
    if algo=="dense_core":     return "%d nodes"%len(pytkcore.dense_core(gb,3,3.0))
    if algo=="stable_core":    return "%d cores"%len(pytkcore.stable_core_nodes(gb,3,2,0.3))
    if algo=="pseudocore":     return "max h = %d"%mx(pytkcore.temporal_h_index(g,8))
    if algo=="persistent_core":return "%d nodes"%len(pytkcore.persistent_core(gb,3,3,5))
    raise ValueError(algo)

def worker(path, algo, binsec):
    global pytkcore
    import pytkcore
    g = pytkcore.load(path)
    s = pytkcore.statistics(g)
    q1, q3 = s.min_time + s.time_span//4, s.min_time + 3*s.time_span//4
    gb=None; S=0
    if algo in SNAPSHOT:
        edges=[]
        for line in open(path):
            line=line.strip()
            if not line or line[0] in "#%": continue
            p=line.split()
            if len(p)<2: continue
            edges.append((int(p[0]),int(p[1]),int(p[2]) if len(p)>=3 else 0))
        tmin=min(t for _,_,t in edges)
        gb=pytkcore.TemporalGraph()
        for u,v,t in edges: gb.add_edge(u,v,(t-tmin)//binsec)
        gb.finalize(); S=pytkcore.num_snapshots(gb)
    t0=time.perf_counter()
    res=run_one(g,gb,S,q1,q3,algo)
    dt=(time.perf_counter()-t0)*1000
    print(json.dumps({"time_ms":dt,"peak_mb":peak_mb(),"result":res,
                      "nodes":g.num_nodes(),"edges":g.num_edges()}))

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--datasets",nargs="+",required=True)
    ap.add_argument("--timeout",type=int,default=300)
    ap.add_argument("--bin",type=int,default=86400)
    ap.add_argument("--out",default="SCALING")
    ap.add_argument("--worker",nargs=3)
    a=ap.parse_args()
    if a.worker:
        worker(a.worker[0],a.worker[1],int(a.worker[2])); return

    rows=[]
    for ds in a.datasets:
        if not os.path.exists(ds):
            print("  MISSING %s -- skipping"%ds,file=sys.stderr); continue
        for algo in ALGOS:
            print("  %-26s %-16s ... "%(os.path.basename(ds),algo),end="",file=sys.stderr,flush=True)
            cmd=[sys.executable,__file__,"--datasets","x","--worker",ds,algo,str(a.bin)]
            try:
                out=subprocess.run(cmd,capture_output=True,text=True,timeout=a.timeout,env={**os.environ})
                if out.returncode==0:
                    d=json.loads(out.stdout.strip().splitlines()[-1])
                    rows.append((ds,d["nodes"],d["edges"],algo,d["result"],"ok",d["time_ms"],d["peak_mb"]))
                    print("ok  %.0f ms  %.0f MB"%(d["time_ms"],d["peak_mb"]),file=sys.stderr)
                else:
                    rows.append((ds,0,0,algo,"","error",0,0)); print("ERROR",file=sys.stderr)
            except subprocess.TimeoutExpired:
                rows.append((ds,0,0,algo,"","timeout",a.timeout*1000,0))
                print("TIMEOUT (>%ds)"%a.timeout,file=sys.stderr)

    import csv
    with open(a.out+".csv","w",newline="") as f:
        w=csv.writer(f); w.writerow(["dataset","nodes","edges","algorithm","result","status","time_ms","peak_mb"])
        for r in rows: w.writerow((os.path.basename(r[0]),)+r[1:])
    with open(a.out+".md","w") as f:
        f.write("# Scaling benchmark\n\nReal SNAP datasets, 1 subprocess per run, %ds timeout.\n\n"%a.timeout)
        f.write("| Dataset | Edges | Algorithm | Result | Status | Time (ms) | Peak (MB) |\n|---|---:|---|---|---|---:|---:|\n")
        for ds,n,e,algo,res,st,tm,mb in rows:
            f.write("| %s | %s | %s | %s | %s | %s | %s |\n"%(
                os.path.basename(ds), e if e else "-", algo, res or "-", st,
                "%.0f"%tm if st in("ok","timeout") else "-", "%.0f"%mb if st=="ok" else "-"))
    print("\nwrote %s.csv and %s.md (%d rows)"%(a.out,a.out,len(rows)),file=sys.stderr)

if __name__=="__main__": main()
