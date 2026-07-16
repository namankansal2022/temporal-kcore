#include "tkcore/span_core.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace tkcore {

namespace {

inline std::uint64_t pack(NodeId a, NodeId b) {
    if (a > b) std::swap(a, b);
    return (std::uint64_t(a) << 32) | b;
}

std::unordered_map<Timestamp, int> index_timestamps(const TemporalGraph& g, int& T) {
    std::vector<Timestamp> ts;
    ts.reserve(g.num_edges());
    for (const auto& e : g.edges()) ts.push_back(e.t);
    std::sort(ts.begin(), ts.end());
    ts.erase(std::unique(ts.begin(), ts.end()), ts.end());
    T = (int)ts.size();
    std::unordered_map<Timestamp, int> tid;
    tid.reserve(ts.size() * 2);
    for (int i = 0; i < T; ++i) tid.emplace(ts[i], i);
    return tid;
}

} // namespace

int num_snapshots(const TemporalGraph& g) {
    int T = 0;
    index_timestamps(g, T);
    return T;
}

std::vector<CoreNum> span_core_numbers(const TemporalGraph& g, int a, int b) {
    const std::size_t n = g.num_nodes();
    int T = 0;
    auto tid = index_timestamps(g, T);
    if (a < 0 || b >= T || a > b) return std::vector<CoreNum>(n, 0);
    const int L = b - a + 1;

    std::unordered_map<std::uint64_t, std::vector<int>> pidx;
    for (const auto& e : g.edges()) {
        if (e.u == e.v) continue;
        pidx[pack(e.u, e.v)].push_back(tid[e.t]);
    }
    std::vector<std::vector<NodeId>> adj(n);
    for (auto& kv : pidx) {
        auto& idx = kv.second;
        std::sort(idx.begin(), idx.end());
        idx.erase(std::unique(idx.begin(), idx.end()), idx.end());
        int cnt = 0;
        for (int i : idx) if (i >= a && i <= b) ++cnt;
        if (cnt == L) {
            NodeId u = (NodeId)(kv.first >> 32), v = (NodeId)(kv.first & 0xffffffffu);
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
    }
    return simple_core_numbers(adj);
}

std::vector<CoreNum> span_core_numbers_reference(const TemporalGraph& g, int a, int b) {
    const std::size_t n = g.num_nodes();
    int T = 0;
    auto tid = index_timestamps(g, T);
    if (a < 0 || b >= T || a > b) return std::vector<CoreNum>(n, 0);

    std::vector<std::set<std::pair<NodeId, NodeId>>> snap(T);
    for (const auto& e : g.edges()) {
        if (e.u == e.v) continue;
        snap[tid[e.t]].insert({std::min(e.u, e.v), std::max(e.u, e.v)});
    }
    std::set<std::pair<NodeId, NodeId>> inter = snap[a];
    for (int i = a + 1; i <= b && !inter.empty(); ++i) {
        std::set<std::pair<NodeId, NodeId>> next;
        for (const auto& p : inter) if (snap[i].count(p)) next.insert(p);
        inter.swap(next);
    }
    std::vector<std::vector<NodeId>> adj(n);
    for (const auto& p : inter) { adj[p.first].push_back(p.second); adj[p.second].push_back(p.first); }
    return simple_core_numbers_reference(adj);
}

} // namespace tkcore
