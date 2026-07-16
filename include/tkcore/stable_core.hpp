#pragma once
//
// stable_core.hpp
// (mu, tau, eps)-stable cores and communities  (temporal SCAN, Qin et al. 2020).
//
// Snapshot t: the simple graph of edges whose timestamp is t.
// Per-snapshot structural similarity of an adjacent pair (u, v):
//   sigma_t(u,v) = (|N_t(u) & N_t(v)| + 2) / sqrt((|N_t(u)|+1)(|N_t(v)|+1))
// u,v are STABLY SIMILAR if sigma_t(u,v) >= eps in at least tau snapshots.
// A node is a (mu,tau,eps)-CORE if it has at least mu stably-similar neighbours.
// Stable communities: connected components of stably-similar cores, with each
// non-core node stably similar to a core attached as a border.
//
#include "tkcore/temporal_graph.hpp"
#include <vector>

namespace tkcore {

struct StableResult {
    std::vector<char> is_core;     // is_core[v] == 1 iff v is a (mu,tau,eps)-core
    std::vector<int>  community;   // community[v] = representative node id, or -1
};

StableResult stable_cores(const TemporalGraph& g, int mu, int tau, double eps);
StableResult stable_cores_reference(const TemporalGraph& g, int mu, int tau, double eps);

} // namespace tkcore
