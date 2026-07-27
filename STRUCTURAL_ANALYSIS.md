# Structural analysis — do the algorithms agree on important nodes?

Cross-algorithm comparison on `data/CollegeMsg.txt` (top-100 nodes for coreness methods; returned sets for cohesive-group methods; overlap coefficient |A&cap;B|/min(|A|,|B|)). See `figures/structural_overlap.png`.

**Set sizes.** temporal-degree (top100): 100, static (top100): 100, (k,h) h=2 (top100): 100, pseudocore (top100): 100, (l,d)-dense set: 401, (t,t)-persistent set: 879, (m,t,e)-stable set: 1187.

**Overlap coefficient matrix.**

| |temporal-degree (top100)|static (top100)|(k,h) h=2 (top100)|pseudocore (top100)|(l,d)-dense set|(t,t)-persistent set|(m,t,e)-stable set|
|---|---|---|---|---|---|---|---|
| temporal-degree (top100) |1.00|0.41|0.42|0.57|0.97|1.00|1.00|
| static (top100) |0.41|1.00|0.88|0.57|0.94|1.00|0.99|
| (k,h) h=2 (top100) |0.42|0.88|1.00|0.63|0.98|1.00|1.00|
| pseudocore (top100) |0.57|0.57|0.63|1.00|0.94|1.00|1.00|
| (l,d)-dense set |0.97|0.94|0.98|0.94|1.00|0.97|0.98|
| (t,t)-persistent set |1.00|1.00|1.00|1.00|0.97|1.00|0.92|
| (m,t,e)-stable set |1.00|0.99|1.00|1.00|0.98|0.92|1.00|

**Findings.**

- Temporal-degree vs. static top-100 overlap = 0.41: the two most basic definitions **substantially disagree** on which nodes are central (41 shared, 59 unique to temporal-degree, 59 unique to static).
- Strongest agreement: temporal-degree (top100) and (t,t)-persistent set (1.00). Weakest: temporal-degree (top100) and static (top100) (0.41).
- 115 nodes are flagged by at least 5 of the 7 methods (a robust structural core the definitions concur on); the rest are method-specific.

This quantifies how algorithmic choice reshapes the structural insight: methods that count repeated interaction, distinct partners, temporal persistence, or reachability surface overlapping but distinct groups from the *same* network.
