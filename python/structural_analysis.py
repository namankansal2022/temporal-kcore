#!/usr/bin/env python3
"""Structural analysis: do different temporal-core algorithms identify the SAME
nodes as structurally important, or different ones?

Coreness methods contribute their top-K nodes; cohesive-group methods contribute
their returned node set. Agreement is measured with the overlap coefficient
|A∩B|/min(|A|,|B|) (controls for differing sizes).

  PYTHONPATH=build python python/structural_analysis.py data/CollegeMsg.txt 86400 100
"""
import sys, csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pytkcore

def topk(d, k):
    return set(n for n,_ in sorted(d.items(), key=lambda kv: kv[1], reverse=True)[:k])

def main():
    path = sys.argv[1]
    binsec = int(sys.argv[2]) if len(sys.argv)>2 else 86400
    K = int(sys.argv[3]) if len(sys.argv)>3 else 100

    g = pytkcore.load(path)
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

    sets = {}
    sets["temporal-degree (top%d)"%K] = topk(pytkcore.core_numbers(g), K)
    sets["static (top%d)"%K]          = topk(pytkcore.static_core_numbers(g), K)
    sets["(k,h) h=2 (top%d)"%K]       = topk(pytkcore.kh_core_numbers(g,2), K)
    sets["pseudocore (top%d)"%K]      = topk(pytkcore.temporal_h_index(g,8), K)
    sets["(l,d)-dense set"]           = set(pytkcore.dense_core(gb,3,3.0))
    sets["(t,t)-persistent set"]      = set(pytkcore.persistent_core(gb,3,3,5))
    sc = pytkcore.stable_communities(gb,3,2,0.3)
    sets["(m,t,e)-stable set"]        = set(sc.keys())

    names = list(sets.keys())
    print("Set sizes:")
    for n in names: print("  %-26s %d nodes"%(n, len(sets[n])))

    def overlap(a,b):
        if not a or not b: return 0.0
        return len(a & b)/min(len(a),len(b))
    M = [[overlap(sets[a],sets[b]) for b in names] for a in names]

    fig,ax=plt.subplots(figsize=(8.5,7))
    im=ax.imshow(M, cmap="YlGnBu", vmin=0, vmax=1)
    ax.set_xticks(range(len(names))); ax.set_yticks(range(len(names)))
    ax.set_xticklabels(names, rotation=40, ha="right", fontsize=8)
    ax.set_yticklabels(names, fontsize=8)
    for i in range(len(names)):
        for j in range(len(names)):
            ax.text(j,i,"%.2f"%M[i][j],ha="center",va="center",
                    fontsize=7,color="black" if M[i][j]<0.6 else "white")
    ax.set_title("Do the algorithms agree on important nodes?\n(overlap coefficient)")
    fig.colorbar(im,fraction=0.046,pad=0.04)
    plt.tight_layout(); plt.savefig("figures/structural_overlap.png",dpi=150); plt.close()

    td = sets["temporal-degree (top%d)"%K]; st = sets["static (top%d)"%K]
    print("\ntemporal-degree vs static (top-%d): shared=%d  uniq-td=%d  uniq-static=%d  overlap=%.2f"%(
        K,len(td&st),len(td-st),len(st-td),overlap(td,st)))
    from collections import Counter
    cnt=Counter()
    for s in sets.values():
        for n in s: cnt[n]+=1
    broad=[n for n,c in cnt.items() if c>=5]
    print("nodes flagged by >=5 of 7 methods: %d"%len(broad))

    with open("STRUCTURAL_ANALYSIS.csv","w",newline="") as f:
        w=csv.writer(f); w.writerow([""]+names)
        for i,a in enumerate(names): w.writerow([a]+["%.3f"%x for x in M[i]])

    pairs=[(M[i][j],names[i],names[j]) for i in range(len(names)) for j in range(i+1,len(names))]
    lo=min(pairs); hi=max(pairs)
    oc=overlap(td,st)
    verdict=("largely agree" if oc>=0.75 else "partially agree" if oc>=0.5 else "substantially disagree")

    with open("STRUCTURAL_ANALYSIS.md","w") as f:
        f.write("# Structural analysis — do the algorithms agree on important nodes?\n\n")
        f.write("Cross-algorithm comparison on `%s` (top-%d nodes for coreness methods; "
                "returned sets for cohesive-group methods; overlap coefficient "
                "|A&cap;B|/min(|A|,|B|)). See `figures/structural_overlap.png`.\n\n"%(path,K))
        f.write("**Set sizes.** "+", ".join("%s: %d"%(n,len(sets[n])) for n in names)+".\n\n")
        f.write("**Overlap coefficient matrix.**\n\n| |"+"|".join(names)+"|\n")
        f.write("|"+"---|"*(len(names)+1)+"\n")
        for i,a in enumerate(names):
            f.write("| %s |"%a+"|".join("%.2f"%x for x in M[i])+"|\n")
        f.write("\n**Findings.**\n\n")
        f.write("- Temporal-degree vs. static top-%d overlap = %.2f: the two most basic "
                "definitions **%s** on which nodes are central (%d shared, %d unique to "
                "temporal-degree, %d unique to static).\n"%(K,oc,verdict,len(td&st),len(td-st),len(st-td)))
        f.write("- Strongest agreement: %s and %s (%.2f). Weakest: %s and %s (%.2f).\n"%(
                hi[1],hi[2],hi[0],lo[1],lo[2],lo[0]))
        f.write("- %d nodes are flagged by at least 5 of the 7 methods (a robust structural "
                "core the definitions concur on); the rest are method-specific.\n"%len(broad))
        f.write("\nThis quantifies how algorithmic choice reshapes the structural insight: "
                "methods that count repeated interaction, distinct partners, temporal "
                "persistence, or reachability surface overlapping but distinct groups from "
                "the *same* network.\n")

    print("\nwrote figures/structural_overlap.png, STRUCTURAL_ANALYSIS.csv, STRUCTURAL_ANALYSIS.md")

if __name__=="__main__": main()
