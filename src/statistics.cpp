#include "tkcore/statistics.hpp"

#include <cstdint>
#include <unordered_set>

namespace tkcore {

GraphStats compute_statistics(const TemporalGraph& g) {
    GraphStats s;
    s.num_nodes          = g.num_nodes();
    s.num_temporal_edges = g.num_edges();
    s.num_distinct_times = g.num_timestamps();
    s.min_time           = g.min_time();
    s.max_time           = g.max_time();
    s.time_span          = g.max_time() - g.min_time();

    // distinct undirected static edges: pack (min,max) node ids into one 64-bit key
    std::unordered_set<std::uint64_t> pairs;
    pairs.reserve(g.num_edges() * 2);
    for (const auto& e : g.edges()) {
        std::uint32_t a = e.u, b = e.v;
        if (a > b) std::swap(a, b);
        std::uint64_t key = (static_cast<std::uint64_t>(a) << 32) | b;
        pairs.insert(key);
    }
    s.num_static_edges = pairs.size();

    // temporal degree distribution summary
    std::size_t max_deg = 0;
    for (NodeId v = 0; v < g.num_nodes(); ++v) {
        std::size_t d = g.temporal_degree(v);
        if (d > max_deg) max_deg = d;
    }
    s.max_temporal_degree = max_deg;
    // sum of temporal degrees = 2 * |E| (each edge contributes to two endpoints)
    s.avg_temporal_degree =
        s.num_nodes ? (2.0 * static_cast<double>(s.num_temporal_edges) /
                       static_cast<double>(s.num_nodes))
                    : 0.0;
    return s;
}

} // namespace tkcore
