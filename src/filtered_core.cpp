#include "tkcore/filtered_core.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_map>

namespace tkcore {

std::vector<CoreNum> simple_core_numbers(const std::vector<std::vector<NodeId>>& adj) {
    const std::size_t n = adj.size();
    std::vector<CoreNum> core(n, 0);
    if (n == 0) return core;

    std::vector<std::uint32_t> deg(n);
    std::uint32_t md = 0;
    for (std::size_t v = 0; v < n; ++v) { deg[v] = (std::uint32_t)adj[v].size(); md = std::max(md, deg[v]); }

    std::vector<std::uint32_t> bin(md + 1, 0);
    for (std::size_t v = 0; v < n; ++v) ++bin[deg[v]];
    std::uint32_t start = 0;
    for (std::uint32_t d = 0; d <= md; ++d) { std::uint32_t num = bin[d]; bin[d] = start; start += num; }

    std::vector<std::uint32_t> vert(n), pos(n);
    for (std::uint32_t v = 0; v < n; ++v) { pos[v] = bin[deg[v]]; vert[pos[v]] = v; ++bin[deg[v]]; }
    for (std::uint32_t d = md; d >= 1; --d) bin[d] = bin[d - 1];
    bin[0] = 0;

    for (std::uint32_t i = 0; i < n; ++i) {
        std::uint32_t v = vert[i];
        core[v] = deg[v];
        for (NodeId u : adj[v]) {
            if (deg[u] > deg[v]) {
                std::uint32_t du = deg[u], pu = pos[u], pw = bin[du], w = vert[pw];
                if (u != w) { pos[u] = pw; vert[pu] = w; pos[w] = pu; vert[pw] = u; }
                ++bin[du]; --deg[u];
            }
        }
    }
    return core;
}

std::vector<CoreNum> simple_core_numbers_reference(const std::vector<std::vector<NodeId>>& adj) {
    const std::size_t n = adj.size();
    std::vector<CoreNum> core(n, 0);
    if (n == 0) return core;
    std::vector<int> deg(n);
    std::vector<char> removed(n, 0);
    for (std::size_t v = 0; v < n; ++v) deg[v] = (int)adj[v].size();
    int level = 0;
    for (std::size_t iter = 0; iter < n; ++iter) {
        int best = -1;
        for (std::size_t v = 0; v < n; ++v)
            if (!removed[v] && (best < 0 || deg[v] < deg[best])) best = (int)v;
        if (best < 0) break;
        level = std::max(level, deg[best]);
        core[best] = level;
        removed[best] = 1;
        for (NodeId u : adj[best]) if (!removed[u]) --deg[u];
    }
    return core;
}

namespace {

std::vector<std::vector<NodeId>> adjacency_from_pairs(
        std::size_t n, const std::unordered_map<std::uint64_t, std::uint32_t>& mult, int min_mult) {
    std::vector<std::vector<NodeId>> adj(n);
    for (const auto& kv : mult) {
        if ((int)kv.second < min_mult) continue;
        NodeId a = (NodeId)(kv.first >> 32);
        NodeId b = (NodeId)(kv.first & 0xffffffffu);
        adj[a].push_back(b);
        adj[b].push_back(a);
    }
    return adj;
}

inline std::uint64_t pack(NodeId a, NodeId b) {
    if (a > b) std::swap(a, b);
    return (std::uint64_t(a) << 32) | b;
}

} // namespace

std::vector<CoreNum> static_core_numbers(const TemporalGraph& g) {
    std::unordered_map<std::uint64_t, std::uint32_t> mult;
    mult.reserve(g.num_edges() * 2);
    for (const auto& e : g.edges()) if (e.u != e.v) mult[pack(e.u, e.v)] = 1;
    return simple_core_numbers(adjacency_from_pairs(g.num_nodes(), mult, 1));
}

std::vector<CoreNum> kh_core_numbers(const TemporalGraph& g, int h) {
    std::unordered_map<std::uint64_t, std::uint32_t> mult;
    mult.reserve(g.num_edges() * 2);
    for (const auto& e : g.edges()) if (e.u != e.v) ++mult[pack(e.u, e.v)];
    return simple_core_numbers(adjacency_from_pairs(g.num_nodes(), mult, std::max(1, h)));
}

std::vector<CoreNum> window_core_numbers(const TemporalGraph& g, Timestamp ts, Timestamp te) {
    std::unordered_map<std::uint64_t, std::uint32_t> mult;
    mult.reserve(g.num_edges() * 2);
    for (const auto& e : g.edges())
        if (e.u != e.v && e.t >= ts && e.t <= te) mult[pack(e.u, e.v)] = 1;
    return simple_core_numbers(adjacency_from_pairs(g.num_nodes(), mult, 1));
}

} // namespace tkcore
