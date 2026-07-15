#pragma once
//
// kcore.hpp
// Temporal-degree k-core decomposition (TODO #3).
//
// Degree model
// ------------
// The core number of a node is computed with respect to its TEMPORAL degree,
// i.e. the number of temporal edges incident to it (parallel temporal edges
// between the same pair of nodes are counted with multiplicity). Equivalently,
// this is the k-core of the underlying temporal multigraph.
//
// A subgraph H is a k-core if every node in H has temporal degree >= k inside
// H, and H is maximal with this property. The core number c(v) is the largest
// k such that v belongs to the k-core.
//
#include "tkcore/temporal_graph.hpp"
#include <cstdint>
#include <vector>

namespace tkcore {

using CoreNumber = std::uint32_t;

// Linear-time O(|V| + |E|) core decomposition via bin-sort peeling
// (Batagelj & Zaversnik, 2003), adapted to the temporal multigraph degree.
// Returns a vector `core` of length num_nodes(); core[v] is the core number of
// internal node v.
std::vector<CoreNumber> temporal_core_numbers(const TemporalGraph& g);

// Independent O(|V| + |E| log |V|) reference implementation using a bucket
// min-queue with explicit multiplicity handling. Used to validate the fast
// algorithm ("accuracy" check). Much simpler, deliberately not optimized.
std::vector<CoreNumber> temporal_core_numbers_reference(const TemporalGraph& g);

} // namespace tkcore
