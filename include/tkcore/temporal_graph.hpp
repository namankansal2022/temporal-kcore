#pragma once
//
// temporal_graph.hpp
// Core data structure for a temporal (multi-)graph.
//
// A temporal graph G = (V, E) is a set of nodes V together with a set of
// temporal edges E = { (u, v, t) }, where each edge carries an integer
// timestamp t indicating when the interaction occurred.
//
// Design notes
// ------------
// * External node ids (as they appear in the dataset) can be arbitrary 64-bit
//   integers. We map them to a contiguous internal range [0, n) so that all
//   per-node data can live in flat vectors (cache friendly, no hashing in the
//   hot loops).
// * We keep the full temporal edge list (lossless), and additionally build an
//   *incidence list* per node. Each temporal edge (u, v, t) contributes ONE
//   entry to u's list and ONE entry to v's list. Therefore
//         temporal_degree(u) = incidence_[u].size()
//   is exactly "the number of temporal edges incident to u" (parallel edges
//   between the same pair of nodes are counted with multiplicity). This is the
//   notion of degree requested for the temporal k-core.
//
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace tkcore {

using NodeId    = std::uint32_t;   // internal contiguous id in [0, n)
using Timestamp = std::int64_t;    // discrete time label

struct TemporalEdge {
    NodeId    u;
    NodeId    v;
    Timestamp t;
};

class TemporalGraph {
public:
    // Insert one temporal edge using ORIGINAL (external) node ids.
    // Node ids are interned lazily; timestamps may arrive in any order.
    void add_edge(std::int64_t src_ext, std::int64_t dst_ext, Timestamp t);

    // Build the per-node incidence lists and cache derived quantities
    // (distinct timestamps, min/max time). Must be called once after all
    // edges have been added; the loader calls it for you.
    void finalize();

    // ---- sizes -------------------------------------------------------------
    std::size_t num_nodes()      const { return id_to_ext_.size(); }
    std::size_t num_edges()      const { return edges_.size(); }        // temporal edges
    std::size_t num_timestamps() const { return num_distinct_ts_; }    // distinct t values

    // ---- access ------------------------------------------------------------
    const std::vector<TemporalEdge>&              edges()     const { return edges_; }
    const std::vector<std::vector<NodeId>>&       incidence() const { return incidence_; }

    // temporal degree = number of temporal edges incident to the node
    std::size_t temporal_degree(NodeId v) const { return incidence_[v].size(); }

    // map back to the original id from the dataset
    std::int64_t external_id(NodeId internal) const { return id_to_ext_[internal]; }

    Timestamp min_time() const { return min_time_; }
    Timestamp max_time() const { return max_time_; }

    bool finalized() const { return finalized_; }

private:
    NodeId intern(std::int64_t ext);

    std::unordered_map<std::int64_t, NodeId> ext_to_id_;
    std::vector<std::int64_t>                id_to_ext_;
    std::vector<TemporalEdge>                edges_;
    std::vector<std::vector<NodeId>>         incidence_;

    std::size_t num_distinct_ts_ = 0;
    Timestamp   min_time_ = 0;
    Timestamp   max_time_ = 0;
    bool        finalized_ = false;
};

} // namespace tkcore
