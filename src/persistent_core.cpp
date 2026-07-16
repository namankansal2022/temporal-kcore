#include "tkcore/persistent_core.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tkcore {

namespace {

inline std::uint64_t pack(NodeId a, NodeId b) {
    if (a > b) std::swap(a, b);
    return (std::uint64_t(a) << 32) | b;
}

int build_snapshots(const TemporalGraph& g,
                    std::vector<std::vector<std::uint64_t>>& snap_pairs) {
    std::vector<Timestamp> ts;
    ts.reserve(g.num_edges());
    for (const auto& e : g.edges()) ts.push_back(e.t);
    std::sort(ts.begin(), ts.end());
    ts.erase(std::unique(ts.begin(), ts.end()), ts.end());
    int T = (int)ts.size();
    std::unordered_map<Timestamp, int> tid;
    tid.reserve(ts.size() * 2);
    for (int i = 0; i < T; ++i) tid.emplace(ts[i], i);

    std::vector<std::unordered_set<std::uint64_t>> sets(T);
    for (const auto& e : g.edges()) {
        if (e.u == e.v) continue;
        sets[tid[e.t]].insert(pack(e.u, e.v));
    }
    snap_pairs.assign(T, {});
    for (int i = 0; i < T; ++i) snap_pairs[i].assign(sets[i].begin(), sets[i].end());
    return T;
}

long long persistence_from_starts(const std::vector<int>& starts, int theta) {
    if (starts.empty()) return 0;
    long long sum_lengths = 0;
    int runs = 0;
    std::size_t i = 0;
    while (i < starts.size()) {
        std::size_t j = i;
        while (j + 1 < starts.size() && starts[j + 1] == starts[j] + 1) ++j;
        sum_lengths += (long long)(starts[j] - starts[i]) + theta;
        ++runs;
        i = j + 1;
    }
    return sum_lengths - (long long)(runs - 1) * theta;
}

template <typename CoreFn>
std::vector<long long> persistence_impl(const TemporalGraph& g, int theta, int k, CoreFn core_fn) {
    const std::size_t n = g.num_nodes();
    std::vector<long long> pers(n, 0);
    if (theta < 0) return pers;

    std::vector<std::vector<std::uint64_t>> snap_pairs;
    int T = build_snapshots(g, snap_pairs);
    if (T < theta + 1) return pers;

    std::vector<std::vector<int>> active(n);
    std::vector<std::vector<NodeId>> adj(n);

    for (int t = 0; t + theta <= T - 1; ++t) {
        std::unordered_set<std::uint64_t> pk;
        for (int i = t; i <= t + theta; ++i)
            for (std::uint64_t key : snap_pairs[i]) pk.insert(key);

        for (auto& a : adj) a.clear();
        for (std::uint64_t key : pk) {
            NodeId a = (NodeId)(key >> 32), b = (NodeId)(key & 0xffffffffu);
            adj[a].push_back(b);
            adj[b].push_back(a);
        }
        std::vector<CoreNum> core = core_fn(adj);
        for (NodeId v = 0; v < n; ++v)
            if ((int)core[v] >= k) active[v].push_back(t);
    }

    for (NodeId v = 0; v < n; ++v)
        pers[v] = persistence_from_starts(active[v], theta);
    return pers;
}

} // namespace

std::vector<long long> persistence_values(const TemporalGraph& g, int theta, int k) {
    return persistence_impl(g, theta, k,
        [](const std::vector<std::vector<NodeId>>& a) { return simple_core_numbers(a); });
}

std::vector<long long> persistence_values_reference(const TemporalGraph& g, int theta, int k) {
    return persistence_impl(g, theta, k,
        [](const std::vector<std::vector<NodeId>>& a) { return simple_core_numbers_reference(a); });
}

std::vector<NodeId> persistent_core(const TemporalGraph& g, int theta, int k, long long tau) {
    auto pers = persistence_values(g, theta, k);
    std::vector<NodeId> out;
    for (NodeId v = 0; v < g.num_nodes(); ++v)
        if (pers[v] >= tau) out.push_back(v);
    return out;
}

} // namespace tkcore
