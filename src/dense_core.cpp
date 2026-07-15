#include "tkcore/dense_core.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <queue>
#include <unordered_map>

namespace tkcore {

namespace {

constexpr double NEG_INF = -std::numeric_limits<double>::infinity();

struct Prepared {
    std::size_t n = 0;
    int T = 0;
    std::vector<std::vector<NodeId>> adj;
    std::vector<std::vector<std::pair<int, NodeId>>> tinc;
};

Prepared prepare(const TemporalGraph& g) {
    Prepared p;
    p.n = g.num_nodes();
    std::vector<Timestamp> ts;
    ts.reserve(g.num_edges());
    for (const auto& e : g.edges()) ts.push_back(e.t);
    std::sort(ts.begin(), ts.end());
    ts.erase(std::unique(ts.begin(), ts.end()), ts.end());
    p.T = static_cast<int>(ts.size());
    std::unordered_map<Timestamp, int> tid;
    tid.reserve(ts.size() * 2);
    for (int i = 0; i < p.T; ++i) tid.emplace(ts[i], i);
    p.tinc.assign(p.n, {});
    for (const auto& e : g.edges()) {
        int s = tid[e.t];
        p.tinc[e.u].push_back({s, e.v});
        p.tinc[e.v].push_back({s, e.u});
    }
    p.adj.assign(p.n, {});
    for (NodeId u = 0; u < p.n; ++u) {
        auto& ti = p.tinc[u];
        std::sort(ti.begin(), ti.end());
        ti.erase(std::unique(ti.begin(), ti.end()), ti.end());
        auto& a = p.adj[u];
        for (const auto& pr : ti) a.push_back(pr.second);
        std::sort(a.begin(), a.end());
        a.erase(std::unique(a.begin(), a.end()), a.end());
    }
    return p;
}

void degree_sequence(const Prepared& p, NodeId u, const std::vector<char>& active,
                     std::vector<int>& ds) {
    std::fill(ds.begin(), ds.end(), 0);
    for (const auto& pr : p.tinc[u])
        if (active[pr.second]) ++ds[pr.first];
}

} // namespace

double max_l_segment_density(const std::vector<int>& a, int l) {
    const int n = static_cast<int>(a.size());
    if (l < 1) l = 1;
    if (n < l) return NEG_INF;
    std::vector<long long> P(n + 1, 0);
    for (int i = 0; i < n; ++i) P[i + 1] = P[i] + a[i];
    auto slope = [&](int i, int j) { return double(P[j] - P[i]) / (j - i); };
    std::deque<int> dq;
    double best = NEG_INF;
    for (int j = l; j <= n; ++j) {
        int c = j - l;
        while (dq.size() >= 2) {
            int A = dq[dq.size() - 2], B = dq[dq.size() - 1];
            long long cross = (long long)(B - A) * (P[c] - P[A]) -
                              (long long)(P[B] - P[A]) * (c - A);
            if (cross <= 0) dq.pop_back(); else break;
        }
        dq.push_back(c);
        while (dq.size() >= 2 && slope(dq[0], j) <= slope(dq[1], j)) dq.pop_front();
        best = std::max(best, slope(dq[0], j));
    }
    return best;
}

double max_l_segment_density_naive(const std::vector<int>& a, int l) {
    const int n = static_cast<int>(a.size());
    if (l < 1) l = 1;
    if (n < l) return NEG_INF;
    std::vector<long long> P(n + 1, 0);
    for (int i = 0; i < n; ++i) P[i + 1] = P[i] + a[i];
    double best = NEG_INF;
    for (int i = 0; i <= n; ++i)
        for (int j = i + l; j <= n; ++j)
            best = std::max(best, double(P[j] - P[i]) / (j - i));
    return best;
}

std::vector<NodeId> dense_core(const TemporalGraph& g, int l, double delta) {
    Prepared p = prepare(g);
    const std::size_t n = p.n;
    std::vector<NodeId> empty;
    if (n == 0 || p.T < l || delta <= 0) return empty;
    const int k = static_cast<int>(std::ceil(delta));
    std::vector<int> deg(n);
    std::vector<char> active(n, 1);
    std::queue<NodeId> q;
    for (NodeId u = 0; u < n; ++u) {
        deg[u] = static_cast<int>(p.adj[u].size());
        if (deg[u] < k) { active[u] = 0; q.push(u); }
    }
    while (!q.empty()) {
        NodeId v = q.front(); q.pop();
        for (NodeId w : p.adj[v])
            if (active[w] && --deg[w] < k) { active[w] = 0; q.push(w); }
    }
    std::vector<int> ds(p.T);
    std::vector<char> removed(n, 0);
    std::queue<NodeId> del;
    for (NodeId u = 0; u < n; ++u) {
        if (!active[u]) { removed[u] = 1; continue; }
        degree_sequence(p, u, active, ds);
        if (max_l_segment_density(ds, l) < delta) del.push(u);
    }
    for (NodeId u = 0; u < n; ++u) {
        if (!active[u]) continue;
        int d = 0;
        for (NodeId w : p.adj[u]) if (active[w]) ++d;
        deg[u] = d;
    }
    while (!del.empty()) {
        NodeId v = del.front(); del.pop();
        if (removed[v]) continue;
        removed[v] = 1; active[v] = 0;
        for (NodeId w : p.adj[v]) {
            if (removed[w] || !active[w]) continue;
            if (--deg[w] < k) { del.push(w); continue; }
            degree_sequence(p, w, active, ds);
            if (max_l_segment_density(ds, l) < delta) del.push(w);
        }
    }
    std::vector<NodeId> result;
    for (NodeId u = 0; u < n; ++u)
        if (active[u] && !removed[u]) result.push_back(u);
    return result;
}

std::vector<NodeId> dense_core_reference(const TemporalGraph& g, int l, double delta) {
    Prepared p = prepare(g);
    const std::size_t n = p.n;
    std::vector<NodeId> empty;
    if (n == 0 || p.T < l || delta <= 0) return empty;
    std::vector<char> active(n, 1);
    std::vector<int> ds(p.T);
    bool changed = true;
    while (changed) {
        changed = false;
        for (NodeId u = 0; u < n; ++u) {
            if (!active[u]) continue;
            degree_sequence(p, u, active, ds);
            if (max_l_segment_density_naive(ds, l) < delta) { active[u] = 0; changed = true; }
        }
    }
    std::vector<NodeId> result;
    for (NodeId u = 0; u < n; ++u) if (active[u]) result.push_back(u);
    return result;
}

} // namespace tkcore
