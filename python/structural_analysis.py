#!/usr/bin/env python3
"""Structural analysis: do different temporal-core algorithms identify the SAME
nodes as structurally important, or different ones?

This is the "how do algorithmic choices affect the structural insight" study.
We compare the seven decompositions three complementary ways, because no single
measure is honest on its own:

  1. Spearman rank correlation (coreness methods only). The four coreness
     methods assign a numeric importance score to EVERY node, so we can compare
     their full orderings rather than a truncated top-K. rho = 1 means identical
     ranking, 0 means unrelated. This is the primary, size-independent measure.

  2. Jaccard index  |A cap B| / |A cup B|  over the reported node sets (top-K for
     coreness methods, returned sets for the cohesive-group methods). Unlike the
     overlap coefficient it is penalised by size differences, so it does not go
     to 1.0 just because a small set sits inside a large one.

  3. Overlap coefficient  |A cap B| / min(|A|,|B|)  is ALSO reported, but only
     with an explicit caveat: whenever a small set is (nearly) a subset of a much
     larger one it inflates to ~1.0 regardless of real agreement. We flag those
     inflated cells rather than reporting them as findings.

Usage (identical CLI to before):
    PYTHONPATH=build python python/structural_analysis.py data/CollegeMsg.txt 86400 100
"""
import sys, csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.stats import spearmanr
import pytkcore


def topk(d, k):
    return set(n for n, _ in sorted(d.items(), key=lambda kv: kv[1], reverse=True)[:k])


def main():
    path = sys.argv[1]
    binsec = int(sys.argv[2]) if len(sys.argv) > 2 else 86400
    K = int(sys.argv[3]) if len(sys.argv) > 3 else 100

    g = pytkcore.load(path)

    edges = []
    for line in open(path):
        line = line.strip()
        if not line or line[0] in "#%":
            continue
        p = line.split()
        if len(p) < 2:
            continue
        edges.append((int(p[0]), int(p[1]), int(p[2]) if len(p) >= 3 else 0))
    tmin = min(t for _, _, t in edges)
    gb = pytkcore.TemporalGraph()
    for u, v, t in edges:
        gb.add_edge(u, v, (t - tmin) // binsec)
    gb.finalize()

    scores = {
        "temporal-degree": pytkcore.core_numbers(g),
        "static":          pytkcore.static_core_numbers(g),
        "(k,h) h=2":       pytkcore.kh_core_numbers(g, 2),
        "pseudocore":      pytkcore.temporal_h_index(g, 8),
    }
    cnames = list(scores.keys())
    all_nodes = sorted(set().union(*[set(d) for d in scores.values()]))
    vecs = {n: [scores[n].get(v, 0) for v in all_nodes] for n in cnames}
    SP = [[spearmanr(vecs[a], vecs[b]).correlation for b in cnames] for a in cnames]

    sets = {}
    sets["temporal-degree (top%d)" % K] = topk(scores["temporal-degree"], K)
    sets["static (top%d)" % K]          = topk(scores["static"], K)
    sets["(k,h) h=2 (top%d)" % K]       = topk(scores["(k,h) h=2"], K)
    sets["pseudocore (top%d)" % K]      = topk(scores["pseudocore"], K)
    sets["(l,d)-dense set"]             = set(pytkcore.dense_core(gb, 3, 3.0))
    sets["(t,t)-persistent set"]        = set(pytkcore.persistent_core(gb, 3, 3, 5))
    sc = pytkcore.stable_communities(gb, 3, 2, 0.3)
    sets["(m,t,e)-stable set"]          = set(sc.keys())
    names = list(sets.keys())

    def jaccard(a, b):
        if not a and not b:
            return 1.0
        return len(a & b) / len(a | b) if (a | b) else 0.0

    def overlap(a, b):
        if not a or not b:
            return 0.0
        return len(a & b) / min(len(a), len(b))

    JA = [[jaccard(sets[a], sets[b]) for b in names] for a in names]
    OV = [[overlap(sets[a], sets[b]) for b in names] for a in names]
    INFL = [[(overlap(sets[a], sets[b]) >= 0.9 and
              max(len(sets[a]), len(sets[b])) >= 3 * max(1, min(len(sets[a]), len(sets[b]))))
             for b in names] for a in names]

    print("Set sizes:")
    for n in names:
        print("  %-26s %d nodes" % (n, len(sets[n])))
    td, st = sets["temporal-degree (top%d)" % K], sets["static (top%d)" % K]
    print("\nSpearman rho (coreness rankings, all nodes):")
    for i, a in enumerate(cnames):
        print("  " + "  ".join("%s/%s=%.2f" % (a, cnames[j], SP[i][j])
                               for j in range(i + 1, len(cnames))))
    print("\ntemporal-degree vs static top-%d: shared=%d jaccard=%.2f overlap=%.2f" %
          (K, len(td & st), jaccard(td, st), overlap(td, st)))
    from collections import Counter
    cnt = Counter()
    for s in sets.values():
        for n in s:
            cnt[n] += 1
    broad = [n for n, c in cnt.items() if c >= 5]
    print("nodes flagged by >=5 of 7 methods: %d" % len(broad))

    fig, (axS, axJ) = plt.subplots(1, 2, figsize=(15, 6.2))
    imS = axS.imshow(SP, cmap="RdBu_r", vmin=-1, vmax=1)
    axS.set_xticks(range(len(cnames))); axS.set_yticks(range(len(cnames)))
    axS.set_xticklabels(cnames, rotation=40, ha="right", fontsize=9)
    axS.set_yticklabels(cnames, fontsize=9)
    for i in range(len(cnames)):
        for j in range(len(cnames)):
            axS.text(j, i, "%.2f" % SP[i][j], ha="center", va="center",
                     fontsize=9, color="white" if abs(SP[i][j]) > 0.55 else "black")
    axS.set_title("Coreness rankings: Spearman rho\n(all nodes, size-independent)")
    fig.colorbar(imS, ax=axS, fraction=0.046, pad=0.04)

    imJ = axJ.imshow(JA, cmap="YlGnBu", vmin=0, vmax=1)
    axJ.set_xticks(range(len(names))); axJ.set_yticks(range(len(names)))
    axJ.set_xticklabels(names, rotation=40, ha="right", fontsize=8)
    axJ.set_yticklabels(names, fontsize=8)
    for i in range(len(names)):
        for j in range(len(names)):
            axJ.text(j, i, "%.2f" % JA[i][j], ha="center", va="center",
                     fontsize=7, color="white" if JA[i][j] > 0.55 else "black")
    axJ.set_title("All methods: Jaccard\n(size-penalised set overlap)")
    fig.colorbar(imJ, ax=axJ, fraction=0.046, pad=0.04)
    plt.tight_layout()
    plt.savefig("figures/structural_overlap.png", dpi=150)
    plt.close()

    with open("STRUCTURAL_ANALYSIS.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["# Spearman rank correlation (coreness methods)"])
        w.writerow([""] + cnames)
        for i, a in enumerate(cnames):
            w.writerow([a] + ["%.3f" % x for x in SP[i]])
        w.writerow([])
        w.writerow(["# Jaccard index (all methods)"])
        w.writerow([""] + names)
        for i, a in enumerate(names):
            w.writerow([a] + ["%.3f" % x for x in JA[i]])
        w.writerow([])
        w.writerow(["# Overlap coefficient (all methods) -- '*' = size-inflated, see .md"])
        w.writerow([""] + names)
        for i, a in enumerate(names):
            w.writerow([a] + ["%.3f%s" % (OV[i][j], "*" if INFL[i][j] else "")
                              for j in range(len(names))])

    sp_pairs = [(SP[i][j], cnames[i], cnames[j])
                for i in range(len(cnames)) for j in range(i + 1, len(cnames))]
    sp_lo, sp_hi = min(sp_pairs), max(sp_pairs)
    ja_pairs = [(JA[i][j], names[i], names[j])
                for i in range(len(names)) for j in range(i + 1, len(names))]
    ja_lo, ja_hi = min(ja_pairs), max(ja_pairs)
    ja_ts = jaccard(td, st)
    verdict = ("largely agree" if sp_lo[0] >= 0.75 else
               "only partially agree" if sp_lo[0] >= 0.4 else
               "diverge substantially")

    with open("STRUCTURAL_ANALYSIS.md", "w") as f:
        f.write("# Structural analysis - how algorithmic choice reshapes the insight\n\n")
        f.write("Do the temporal-core definitions flag the **same** nodes as important, "
                "or different ones? Comparison on `%s` (bin = %ds; top-%d nodes for the "
                "coreness methods, returned sets for the cohesive-group methods). "
                "See `figures/structural_overlap.png`.\n\n" % (path, binsec, K))
        f.write("We use three measures deliberately, because each alone is misleading:\n\n")
        f.write("- **Spearman rank correlation** (coreness methods): compares the full "
                "per-node importance *ordering*, so it is independent of the top-K cutoff "
                "and of set size. This is the primary measure.\n")
        f.write("- **Jaccard** `|A n B|/|A u B|` (all methods): set overlap that is "
                "penalised by size differences.\n")
        f.write("- **Overlap coefficient** `|A n B|/min(|A|,|B|)`: reported for "
                "completeness but **inflates to ~1.0 whenever a small set is nearly a "
                "subset of a much larger one** - those cells are marked `*` in the CSV and "
                "are *not* treated as agreement.\n\n")
        f.write("**Set sizes.** " + ", ".join("%s: %d" % (n, len(sets[n])) for n in names) + ".\n\n")
        f.write("## 1. Spearman rank correlation - coreness rankings\n\n")
        f.write("| |" + "|".join(cnames) + "|\n")
        f.write("|" + "---|" * (len(cnames) + 1) + "\n")
        for i, a in enumerate(cnames):
            f.write("| %s |" % a + "|".join("%.2f" % x for x in SP[i]) + "|\n")
        f.write("\n## 2. Jaccard - all methods\n\n")
        f.write("| |" + "|".join(names) + "|\n")
        f.write("|" + "---|" * (len(names) + 1) + "\n")
        for i, a in enumerate(names):
            f.write("| %s |" % a + "|".join("%.2f" % x for x in JA[i]) + "|\n")
        f.write("\n## Findings\n\n")
        f.write("- **The definition of \"degree\" dominates.** temporal-degree vs. static "
                "coreness rank the nodes with Spearman rho = %.2f and their top-%d sets "
                "have Jaccard = %.2f (%d shared, %d unique each) - the two most basic "
                "definitions **%s**. Counting *repeated interactions* (temporal-degree) "
                "rewards a different set of nodes than counting *distinct partners* "
                "(static): hyper-active accounts with a few heavily-repeated contacts rise "
                "under the former but not the latter.\n"
                % (SP[cnames.index("temporal-degree")][cnames.index("static")],
                   K, ja_ts, len(td & st), len(td - st), verdict))
        f.write("- **(k,h) tracks static; pseudocore is the smoothed hybrid.** Strongest "
                "ranking agreement: %s and %s (rho = %.2f). Weakest: %s and %s "
                "(rho = %.2f). (k,h)-core (which also thresholds on partner count) sits "
                "close to static, while the (eta,k)-pseudocore - an iterated local H-index "
                "- correlates moderately with both, behaving as a smoothed compromise.\n"
                % (sp_hi[1], sp_hi[2], sp_hi[0], sp_lo[1], sp_lo[2], sp_lo[0]))
        f.write("- **Cohesive-group methods surface a broader periphery, not just hubs.** "
                "By Jaccard the reported node sets overlap most for %s and %s (%.2f) and "
                "least for %s and %s (%.2f). The dense/persistent/stable methods return "
                "hundreds of nodes - they capture *sustained group activity*, which "
                "includes many mid-activity nodes the coreness top-K never reaches. (The "
                "overlap coefficient hides this by scoring those pairs ~1.0; Jaccard does "
                "not.)\n" % (ja_hi[1], ja_hi[2], ja_hi[0], ja_lo[1], ja_lo[2], ja_lo[0]))
        f.write("- **A robust structural core exists underneath the disagreement.** %d "
                "nodes are flagged by at least 5 of the 7 methods - a consensus core all "
                "definitions concur on - while the remainder are method-specific. So "
                "algorithmic choice does not randomise the answer; it re-weights a stable "
                "backbone by *which kind of temporal cohesion* it privileges.\n\n" % len(broad))
        f.write("**Takeaway.** From one network, the choice of definition changes *which "
                "nodes look important* in a structured, interpretable way - repeat-contact "
                "intensity vs. partner diversity vs. sustained-group membership - rather "
                "than by measurement noise. Reporting Spearman and Jaccard (not the overlap "
                "coefficient) is what makes that visible.\n")

    print("\nwrote figures/structural_overlap.png, STRUCTURAL_ANALYSIS.csv, STRUCTURAL_ANALYSIS.md")


if __name__ == "__main__":
    main()
