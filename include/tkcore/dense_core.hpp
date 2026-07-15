#pragma once
//
// dense_core.hpp
// (l, delta)-maximal dense core  ("bursting core", Qin et al., VLDB 2022).
//
#include "tkcore/temporal_graph.hpp"
#include <cstdint>
#include <vector>

namespace tkcore {

// Maximum l-segment density of a degree sequence: the largest average value
// over all contiguous segments of length >= l. O(n) via a lower-convex-hull sweep.
double max_l_segment_density(const std::vector<int>& degree_seq, int l);
// Naive O(n^2) version, used as a correctness reference.
double max_l_segment_density_naive(const std::vector<int>& degree_seq, int l);

// Compute the (l, delta)-maximal dense core. Returns internal node ids (empty if none).
std::vector<NodeId> dense_core(const TemporalGraph& g, int l, double delta);

// Independent reference: drop any node whose (naive) MSD < delta until stable.
std::vector<NodeId> dense_core_reference(const TemporalGraph& g, int l, double delta);

} // namespace tkcore
