#include "tkcore/temporal_graph.hpp"

#include <algorithm>

namespace tkcore {

NodeId TemporalGraph::intern(std::int64_t ext) {
    auto it = ext_to_id_.find(ext);
    if (it != ext_to_id_.end()) return it->second;
    NodeId id = static_cast<NodeId>(id_to_ext_.size());
    ext_to_id_.emplace(ext, id);
    id_to_ext_.push_back(ext);
    return id;
}

void TemporalGraph::add_edge(std::int64_t src_ext, std::int64_t dst_ext, Timestamp t) {
    NodeId u = intern(src_ext);
    NodeId v = intern(dst_ext);
    edges_.push_back(TemporalEdge{u, v, t});
    finalized_ = false;
}

void TemporalGraph::finalize() {
    const std::size_t n = num_nodes();

    // --- build incidence lists (undirected: both endpoints get an entry) ----
    incidence_.assign(n, {});
    // Reserve to avoid repeated reallocation: first count, then fill.
    std::vector<std::size_t> deg(n, 0);
    for (const auto& e : edges_) {
        ++deg[e.u];
        ++deg[e.v];
    }
    for (std::size_t i = 0; i < n; ++i) incidence_[i].reserve(deg[i]);
    for (const auto& e : edges_) {
        incidence_[e.u].push_back(e.v);
        incidence_[e.v].push_back(e.u);
    }

    // --- min / max time and distinct timestamp count ------------------------
    if (!edges_.empty()) {
        min_time_ = max_time_ = edges_.front().t;
        std::vector<Timestamp> ts;
        ts.reserve(edges_.size());
        for (const auto& e : edges_) {
            ts.push_back(e.t);
            if (e.t < min_time_) min_time_ = e.t;
            if (e.t > max_time_) max_time_ = e.t;
        }
        std::sort(ts.begin(), ts.end());
        ts.erase(std::unique(ts.begin(), ts.end()), ts.end());
        num_distinct_ts_ = ts.size();
    } else {
        min_time_ = max_time_ = 0;
        num_distinct_ts_ = 0;
    }

    finalized_ = true;
}

} // namespace tkcore
