#include "tkcore/pseudocore.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <tuple>
#include <vector>

namespace tkcore {

namespace {

constexpr Timestamp LAMBDA = 1;
constexpr Timestamp NEG_INF = std::numeric_limits<Timestamp>::min();

struct Trans { Timestamp dep; NodeId w; Timestamp arr; };

std::vector<std::vector<Trans>> build_transitions(const TemporalGraph& g) {
    std::vector<std::vector<Trans>> out(g.num_nodes());
    for (const auto& e : g.edges()) {
        if (e.u == e.v) continue;
        out[e.u].push_back({e.t, e.v, e.t + LAMBDA});
        out[e.v].push_back({e.t, e.u, e.t + LAMBDA});
    }
    for (auto& v : out)
        std::sort(v.begin(), v.end(), [](const Trans& a, const Trans& b) { return a.dep < b.dep; });
    return out;
}

std::uint32_t Hop(std::vector<std::uint32_t>& s) {
    std::sort(s.begin(), s.end(), std::greater<std::uint32_t>());
    std::uint32_t h = 0;
    for (std::uint32_t i = 0; i < s.size(); ++i) {
        if (s[i] >= i + 1) h = i + 1; else break;
    }
    return h;
}

} // namespace

std::vector<std::uint32_t> temporal_h_index(const TemporalGraph& g, int n) {
    const std::size_t N = g.num_nodes();
    auto out = build_transitions(g);
    std::map<std::tuple<int, NodeId, Timestamp>, std::uint32_t> memo;

    std::function<std::uint32_t(int, NodeId, Timestamp)> h =
        [&](int i, NodeId v, Timestamp theta) -> std::uint32_t {
        auto key = std::make_tuple(i, v, theta);
        auto it = memo.find(key);
        if (it != memo.end()) return it->second;
        const auto& tr = out[v];
        std::size_t lo = std::lower_bound(tr.begin(), tr.end(), theta,
            [](const Trans& a, Timestamp t) { return a.dep < t; }) - tr.begin();
        std::uint32_t result;
        if (i == 0) {
            result = (std::uint32_t)(tr.size() - lo);
        } else {
            std::vector<std::uint32_t> vals;
            vals.reserve(tr.size() - lo);
            for (std::size_t j = lo; j < tr.size(); ++j)
                vals.push_back(h(i - 1, tr[j].w, tr[j].arr));
            result = Hop(vals);
        }
        memo.emplace(key, result);
        return result;
    };

    std::vector<std::uint32_t> res(N);
    for (NodeId v = 0; v < N; ++v) res[v] = h(n, v, NEG_INF);
    return res;
}

std::vector<std::uint32_t> temporal_h_index_reference(const TemporalGraph& g, int n) {
    const std::size_t N = g.num_nodes();
    auto out = build_transitions(g);
    std::function<std::uint32_t(int, NodeId, Timestamp)> h =
        [&](int i, NodeId v, Timestamp theta) -> std::uint32_t {
        const auto& tr = out[v];
        std::size_t lo = std::lower_bound(tr.begin(), tr.end(), theta,
            [](const Trans& a, Timestamp t) { return a.dep < t; }) - tr.begin();
        if (i == 0) return (std::uint32_t)(tr.size() - lo);
        std::vector<std::uint32_t> vals;
        for (std::size_t j = lo; j < tr.size(); ++j)
            vals.push_back(h(i - 1, tr[j].w, tr[j].arr));
        return Hop(vals);
    };
    std::vector<std::uint32_t> res(N);
    for (NodeId v = 0; v < N; ++v) res[v] = h(n, v, NEG_INF);
    return res;
}

std::vector<NodeId> pseudocore(const TemporalGraph& g, int n, int k) {
    auto h = temporal_h_index(g, n);
    std::vector<NodeId> out;
    for (NodeId v = 0; v < g.num_nodes(); ++v)
        if ((int)h[v] >= k) out.push_back(v);
    return out;
}

} // namespace tkcore
