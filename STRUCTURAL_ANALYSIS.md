# Structural analysis - how algorithmic choice reshapes the insight

Do the temporal-core definitions flag the **same** nodes as important, or different ones? Comparison on `data/CollegeMsg.txt` (bin = 86400s; top-100 nodes for the coreness methods, returned sets for the cohesive-group methods). See `figures/structural_overlap.png`.

We use three measures deliberately, because each alone is misleading:

- **Spearman rank correlation** (coreness methods): compares the full per-node importance *ordering*, so it is independent of the top-K cutoff and of set size. This is the primary measure.
- **Jaccard** `|A n B|/|A u B|` (all methods): set overlap that is penalised by size differences.
- **Overlap coefficient** `|A n B|/min(|A|,|B|)`: reported for completeness but **inflates to ~1.0 whenever a small set is nearly a subset of a much larger one** - those cells are marked `*` in the CSV and are *not* treated as agreement.

**Set sizes.** temporal-degree (top100): 100, static (top100): 100, (k,h) h=2 (top100): 100, pseudocore (top100): 100, (l,d)-dense set: 401, (t,t)-persistent set: 879, (m,t,e)-stable set: 1187.

## 1. Spearman rank correlation - coreness rankings

| |temporal-degree|static|(k,h) h=2|pseudocore|
|---|---|---|---|---|
| temporal-degree |1.00|0.96|0.96|0.97|
| static |0.96|1.00|0.92|0.94|
| (k,h) h=2 |0.96|0.92|1.00|0.94|
| pseudocore |0.97|0.94|0.94|1.00|

## 2. Jaccard - all methods

| |temporal-degree (top100)|static (top100)|(k,h) h=2 (top100)|pseudocore (top100)|(l,d)-dense set|(t,t)-persistent set|(m,t,e)-stable set|
|---|---|---|---|---|---|---|---|
| temporal-degree (top100) |1.00|0.26|0.27|0.40|0.24|0.11|0.08|
| static (top100) |0.26|1.00|0.79|0.40|0.23|0.11|0.08|
| (k,h) h=2 (top100) |0.27|0.79|1.00|0.46|0.24|0.11|0.08|
| pseudocore (top100) |0.40|0.40|0.46|1.00|0.23|0.11|0.08|
| (l,d)-dense set |0.24|0.23|0.24|0.23|1.00|0.44|0.33|
| (t,t)-persistent set |0.11|0.11|0.11|0.11|0.44|1.00|0.64|
| (m,t,e)-stable set |0.08|0.08|0.08|0.08|0.33|0.64|1.00|

## Findings

- **The definition of "degree" dominates.** temporal-degree vs. static coreness rank the nodes with Spearman rho = 0.96 and their top-100 sets have Jaccard = 0.26 (41 shared, 59 unique each) - the two most basic definitions **largely agree**. Counting *repeated interactions* (temporal-degree) rewards a different set of nodes than counting *distinct partners* (static): hyper-active accounts with a few heavily-repeated contacts rise under the former but not the latter.
- **(k,h) tracks static; pseudocore is the smoothed hybrid.** Strongest ranking agreement: temporal-degree and pseudocore (rho = 0.97). Weakest: static and (k,h) h=2 (rho = 0.92). (k,h)-core (which also thresholds on partner count) sits close to static, while the (eta,k)-pseudocore - an iterated local H-index - correlates moderately with both, behaving as a smoothed compromise.
- **Cohesive-group methods surface a broader periphery, not just hubs.** By Jaccard the reported node sets overlap most for static (top100) and (k,h) h=2 (top100) (0.79) and least for static (top100) and (m,t,e)-stable set (0.08). The dense/persistent/stable methods return hundreds of nodes - they capture *sustained group activity*, which includes many mid-activity nodes the coreness top-K never reaches. (The overlap coefficient hides this by scoring those pairs ~1.0; Jaccard does not.)
- **A robust structural core exists underneath the disagreement.** 115 nodes are flagged by at least 5 of the 7 methods - a consensus core all definitions concur on - while the remainder are method-specific. So algorithmic choice does not randomise the answer; it re-weights a stable backbone by *which kind of temporal cohesion* it privileges.

**Takeaway.** From one network, the choice of definition changes *which nodes look important* in a structured, interpretable way - repeat-contact intensity vs. partner diversity vs. sustained-group membership - rather than by measurement noise. Reporting Spearman and Jaccard (not the overlap coefficient) is what makes that visible.
