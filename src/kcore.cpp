#include "tkcore/kcore.hpp"

#include <algorithm>
#include <queue>
#include <utility>

namespace tkcore {

// ---------------------------------------------------------------------------
// Fast linear-time peeling  (Batagelj & Zaversnik, 2003)
// ---------------------------------------------------------------------------
//
// The classic algorithm keeps vertices bin-sorted by current degree and peels
// them in nondecreasing order, repairing the ordering in O(1) per edge. We use
// the incidence lists built by TemporalGraph, in which a node connected by m
// parallel temporal edges appears m times in the neighbour list -- so degrees
// are temporal (multi-edge) degrees automatically, and each parallel edge is
// relaxed independently.
//
std::vector<CoreNumber> temporal_core_numbers(const TemporalGraph& g) {
    const std::size_t n = g.num_nodes();
    const auto& inc = g.incidence();

    std::vector<CoreNumber> core(n, 0);
    if (n == 0) return core;

    // current degree of each vertex (temporal degree to start)
    std::vector<std::uint32_t> deg(n);
    std::uint32_t md = 0; // maximum degree
    for (std::size_t v = 0; v < n; ++v) {
        deg[v] = static_cast<std::uint32_t>(inc[v].size());
        md = std::max(md, deg[v]);
    }

    // bin[d] will hold the start position (in vert) of the block of vertices
    // with current degree d.
    std::vector<std::uint32_t> bin(md + 1, 0);
    for (std::size_t v = 0; v < n; ++v) ++bin[deg[v]];

    // prefix sums -> starting index of each degree block
    std::uint32_t start = 0;
    for (std::uint32_t d = 0; d <= md; ++d) {
        std::uint32_t num = bin[d];
        bin[d] = start;
        start += num;
    }

    // vert: vertices sorted by degree;  pos: position of each vertex in vert
    std::vector<std::uint32_t> vert(n), pos(n);
    for (std::uint32_t v = 0; v < n; ++v) {
        pos[v] = bin[deg[v]];
        vert[pos[v]] = v;
        ++bin[deg[v]];
    }
    // restore bin[] starting positions (shifted right by one during the fill)
    for (std::uint32_t d = md; d >= 1; --d) bin[d] = bin[d - 1];
    bin[0] = 0;

    // main peeling loop: process vertices in nondecreasing current degree
    for (std::uint32_t i = 0; i < n; ++i) {
        std::uint32_t v = vert[i];
        core[v] = deg[v]; // running level is nondecreasing => this is the core number

        // "remove" v: relax every incident temporal edge
        for (NodeId u : inc[v]) {
            if (deg[u] > deg[v]) {
                std::uint32_t du = deg[u];
                std::uint32_t pu = pos[u];
                std::uint32_t pw = bin[du];    // first position of block du
                std::uint32_t w  = vert[pw];
                if (u != w) {                  // swap u to the front of its block
                    pos[u] = pw; vert[pu] = w;
                    pos[w] = pu; vert[pw] = u;
                }
                ++bin[du];                     // shrink block du from the left
                --deg[u];                      // u moves one bin left
            }
        }
    }
    return core;
}

// ---------------------------------------------------------------------------
// Reference implementation  (independent, for correctness validation)
// ---------------------------------------------------------------------------
//
// Straightforward peeling with a lazy min-priority-queue keyed by current
// degree. Multiplicity is handled explicitly: when a node is removed, each of
// its incident temporal edges decrements the live endpoint by exactly one.
// Deliberately simple; O(|V| + |E| log |V|).
//
std::vector<CoreNumber> temporal_core_numbers_reference(const TemporalGraph& g) {
    const std::size_t n = g.num_nodes();
    const auto& inc = g.incidence();

    std::vector<CoreNumber> core(n, 0);
    if (n == 0) return core;

    std::vector<std::int64_t> deg(n);
    for (std::size_t v = 0; v < n; ++v)
        deg[v] = static_cast<std::int64_t>(inc[v].size());

    using PQItem = std::pair<std::int64_t, NodeId>; // (degree, node)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;
    for (std::size_t v = 0; v < n; ++v)
        pq.emplace(deg[v], static_cast<NodeId>(v));

    std::vector<char> removed(n, 0);
    std::int64_t level = 0;

    while (!pq.empty()) {
        auto [d, v] = pq.top();
        pq.pop();
        if (removed[v]) continue;        // stale entry
        if (d != deg[v]) continue;       // outdated degree, skip

        if (deg[v] > level) level = deg[v];
        core[v] = static_cast<CoreNumber>(level);
        removed[v] = 1;

        for (NodeId u : inc[v]) {
            if (!removed[u]) {
                --deg[u];                // one temporal edge removed
                pq.emplace(deg[u], u);   // push refreshed key
            }
        }
    }
    return core;
}

} // namespace tkcore
