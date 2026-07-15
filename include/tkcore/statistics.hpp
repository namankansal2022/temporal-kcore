#pragma once
//
// statistics.hpp
// Basic descriptive statistics of a temporal graph (TODO #2).
//
#include "tkcore/temporal_graph.hpp"
#include <cstddef>

namespace tkcore {

struct GraphStats {
    std::size_t num_nodes            = 0;  // |V|
    std::size_t num_temporal_edges   = 0;  // |E|  (edges with multiplicity/time)
    std::size_t num_static_edges     = 0;  // distinct undirected {u,v} pairs
    std::size_t num_distinct_times   = 0;  // number of distinct timestamps
    Timestamp   min_time             = 0;
    Timestamp   max_time             = 0;
    Timestamp   time_span            = 0;  // max_time - min_time
    double      avg_temporal_degree  = 0.0;
    std::size_t max_temporal_degree  = 0;
};

// Compute the statistics above. num_static_edges requires deduplicating the
// (min(u,v), max(u,v)) pairs and costs O(|E|) time / O(#distinct pairs) memory.
GraphStats compute_statistics(const TemporalGraph& g);

} // namespace tkcore
