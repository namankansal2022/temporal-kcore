#!/usr/bin/env python3
"""Benchmark every implemented temporal k-core algorithm on one dataset.

Usage: PYTHONPATH=build python python/benchmark_all.py <edgelist> [bin_seconds]

Raw timestamps drive the degree/peeling cores; a binned graph (default 1 day)
drives the snapshot-based cores (dense, stable, span). Output is Markdown.
"""
import sys, time, pytkcore

def timed(fn):
    t0=time.time(); r=fn(); return r,(time.time()-t0)*1000

def maxval(d): return max(d.values()) if d else 0

def main():
    path=sys.argv[1]; binsec=int(sys.argv[2]) if len(sys.argv)>2 else 86400
    g=pytkcore.load(path); s=pytkcore.statistics(g)

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
    gb.finalize()
    S=pytkcore.num_snapshots(gb)
    q1,q3=tmin+s.time_span//4, tmin+3*s.time_span//4

    rows=[]
    r,ms=timed(lambda: pytkcore.core_numbers(g));             rows.append(("temporal-degree k-core","max core = %d"%maxval(r),ms))
    r,ms=timed(lambda: pytkcore.static_core_numbers(g));      rows.append(("static k-core","max core = %d"%maxval(r),ms))
    r,ms=timed(lambda: pytkcore.kh_core_numbers(g,2));        rows.append(("(k,h)-core, h=2","max core = %d"%maxval(r),ms))
    r,ms=timed(lambda: pytkcore.window_core_numbers(g,q1,q3));rows.append(("time-window k-core (mid 50%)","max core = %d"%maxval(r),ms))
    b=min(4,S-1)
    r,ms=timed(lambda: pytkcore.span_core_numbers(gb,0,b));   rows.append(("span-core [0,%d]"%b,"max core = %d"%maxval(r),ms))
    r,ms=timed(lambda: pytkcore.dense_core(gb,3,3.0));        rows.append(("(l,delta)-dense, l=3 delta=3","%d nodes"%len(r),ms))
    r,ms=timed(lambda: pytkcore.stable_core_nodes(gb,3,2,0.3));rows.append(("(mu,tau,eps)-stable, 3/2/0.3","%d cores"%len(r),ms))
    h,ms=timed(lambda: pytkcore.temporal_h_index(g,8))
    size=sum(1 for v in h.values() if v>=10)
    rows.append(("(n,k)-pseudocore, n=8","max h = %d; %d nodes @ k=10"%(maxval(h),size),ms))

    print("# Benchmark results\n")
    print(f"**Dataset:** `{path}` — {s.num_nodes} nodes, {s.num_temporal_edges} temporal edges, "
          f"{s.num_static_edges} static edges, {s.num_distinct_times} timestamps, "
          f"~{s.time_span/86400:.0f} day span. Binned to {binsec}s -> {S} snapshots.\n")
    print("| Algorithm | Result | Time (ms) |")
    print("|---|---|---:|")
    for name,res,ms in rows:
        print(f"| {name} | {res} | {ms:.1f} |")

if __name__=="__main__": main()
