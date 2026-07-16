#pragma once
//
// persistent_core.hpp
// (theta, tau)-persistent k-core -- POLYNOMIAL decomposition version.
// (based on Li et al., ICDE 2018)
//
// Over snapshots 0..T-1, a length-theta window [t, t+theta] covers snapshots
// t..t+theta. A vertex is "active" at start t if it lies in the ordinary k-core
// of the projected graph over that window. Persistence F(theta,k) of a vertex is
// computed from its maximal runs of consecutive active starts:
//   F = sum_i (te_i - ts_i) - (r-1)*theta.
// The (theta, tau)-persistent core = vertices with persistence >= tau.
//
// SCOPE: this is the polynomial per-vertex decomposition, NOT the NP-hard
// "largest (theta,tau)-persistent k-core" community search; it also relaxes the
// original model's joint-connectivity requirement (uses per-window k-core
// membership). It is the tractable counterpart to the other decompositions here.
//
#include "tkcore/temporal_graph.hpp"
#include "tkcore/filtered_core.hpp"
#include <vector>

namespace tkcore {

std::vector<long long> persistence_values(const TemporalGraph& g, int theta, int k);
std::vector<long long> persistence_values_reference(const TemporalGraph& g, int theta, int k);
std::vector<NodeId> persistent_core(const TemporalGraph& g, int theta, int k, long long tau);

} // namespace tkcore
