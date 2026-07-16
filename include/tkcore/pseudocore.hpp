#pragma once
//
// pseudocore.hpp
// (n, k)-pseudocore via the n-th order temporal H-index
// (Oettershagen, Kriege & Mutzel, KDD 2023).
//
// H(S) = largest i such that at least i elements of multiset S are >= i.
// Temporal neighbourhood N(v,t): multiset of (w, t+lambda) for every temporal
// edge incident to v with departure >= t (undirected transitions; lambda = 1).
//   h^(0)_{v,t} = |N(v,t)|
//   h^(n)_{v,t} = H({ h^(n-1)_{w,t_w} : (w,t_w) in N(v,t) })
//   h^(n)_v     = h^(n)_{v,0}
// The (n,k)-pseudocore is the set of nodes with h^(n)_v >= k.
//
#include "tkcore/temporal_graph.hpp"
#include <cstdint>
#include <vector>

namespace tkcore {

std::vector<std::uint32_t> temporal_h_index(const TemporalGraph& g, int n);
std::vector<std::uint32_t> temporal_h_index_reference(const TemporalGraph& g, int n);
std::vector<NodeId> pseudocore(const TemporalGraph& g, int n, int k);

} // namespace tkcore
