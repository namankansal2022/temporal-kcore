#include "tkcore/stable_core.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>

namespace tkcore {

namespace {

int intersize(const std::vector<NodeId>& A, const std::vector<NodeId>& B) {
    std::size_t i = 0, j = 0; int c = 0;
    while (i < A.size() && j < B.size()) {
        if (A[i] == B[j]) { ++c; ++i; ++j; }
        else if (A[i] < B[j]) ++i; else ++j;
    }
    return c;
}

int uf_find(std::vector<int>& uf, int x) {
    while (uf[x] != x) { uf[x] = uf[uf[x]]; x = uf[x]; }
    return x;
}

} // namespace

StableResult stable_cores(const TemporalGraph& g, int mu, int tau, double eps) {
    const std::size_t n = g.num_nodes();
    StableResult R;
    R.is_core.assign(n, 0);
    R.community.assign(n, -1);
    if (n == 0) return R;

    std::set<std::tuple<NodeId, NodeId, Timestamp>> ded;
    for (const auto& e : g.edges()) {
        if (e.u == e.v) continue;
        ded.insert(std::make_tuple(std::min(e.u, e.v), std::max(e.u, e.v), e.t));
    }
    std::unordered_map<Timestamp, std::unordered_map<NodeId, std::vector<NodeId>>> snap;
    std::map<std::pair<NodeId, NodeId>, std::vector<Timestamp>> pairs;
    for (const auto& tp : ded) {
        NodeId a = std::get<0>(tp), b = std::get<1>(tp); Timestamp t = std::get<2>(tp);
        snap[t][a].push_back(b);
        snap[t][b].push_back(a);
        pairs[{a, b}].push_back(t);
    }
    for (auto& kv : snap) for (auto& p : kv.second)
        std::sort(p.second.begin(), p.second.end());

    std::vector<int> simdeg(n, 0);
    std::map<std::pair<NodeId, NodeId>, bool> stable;
    for (auto& kv : pairs) {
        NodeId a = kv.first.first, b = kv.first.second;
        if ((int)kv.second.size() < tau) { stable[kv.first] = false; continue; }
        int cnt = 0;
        for (Timestamp t : kv.second) {
            const auto& Sa = snap[t][a];
            const auto& Sb = snap[t][b];
            double la = Sa.size() + 1.0, lb = Sb.size() + 1.0;
            double common = intersize(Sa, Sb) + 2.0;
            if (common >= eps * std::sqrt(la * lb)) ++cnt;
        }
        bool s = cnt >= tau;
        stable[kv.first] = s;
        if (s) { ++simdeg[a]; ++simdeg[b]; }
    }

    for (std::size_t i = 0; i < n; ++i)
        if (simdeg[i] >= mu) R.is_core[i] = 1;

    std::vector<int> uf(n);
    for (std::size_t i = 0; i < n; ++i) uf[i] = (int)i;
    for (auto& kv : stable) if (kv.second) {
        NodeId a = kv.first.first, b = kv.first.second;
        if (R.is_core[a] && R.is_core[b]) uf[uf_find(uf, a)] = uf_find(uf, b);
    }
    for (auto& kv : stable) if (kv.second) {
        NodeId a = kv.first.first, b = kv.first.second;
        if (R.is_core[a] && !R.is_core[b] && R.community[b] == -1) R.community[b] = uf_find(uf, a);
        if (R.is_core[b] && !R.is_core[a] && R.community[a] == -1) R.community[a] = uf_find(uf, b);
    }
    for (std::size_t i = 0; i < n; ++i)
        if (R.is_core[i]) R.community[i] = uf_find(uf, (int)i);
    return R;
}

StableResult stable_cores_reference(const TemporalGraph& g, int mu, int tau, double eps) {
    const std::size_t n = g.num_nodes();
    StableResult R;
    R.is_core.assign(n, 0);
    R.community.assign(n, -1);
    if (n == 0) return R;

    std::map<Timestamp, std::map<NodeId, std::set<NodeId>>> snap;
    std::map<std::pair<NodeId, NodeId>, std::set<Timestamp>> pt;
    for (const auto& e : g.edges()) {
        if (e.u == e.v) continue;
        NodeId a = std::min(e.u, e.v), b = std::max(e.u, e.v);
        snap[e.t][a].insert(b);
        snap[e.t][b].insert(a);
        pt[{a, b}].insert(e.t);
    }
    std::vector<int> simdeg(n, 0);
    std::map<std::pair<NodeId, NodeId>, bool> stable;
    for (auto& kv : pt) {
        NodeId a = kv.first.first, b = kv.first.second;
        if ((int)kv.second.size() < tau) { stable[kv.first] = false; continue; }
        int c = 0;
        for (Timestamp t : kv.second) {
            const auto& Sa = snap[t][a];
            const auto& Sb = snap[t][b];
            std::vector<NodeId> I;
            std::set_intersection(Sa.begin(), Sa.end(), Sb.begin(), Sb.end(), std::back_inserter(I));
            double la = Sa.size() + 1.0, lb = Sb.size() + 1.0, common = I.size() + 2.0;
            if (common >= eps * std::sqrt(la * lb)) ++c;
        }
        bool s = c >= tau;
        stable[kv.first] = s;
        if (s) { ++simdeg[a]; ++simdeg[b]; }
    }
    for (std::size_t i = 0; i < n; ++i) if (simdeg[i] >= mu) R.is_core[i] = 1;

    std::map<int, std::vector<int>> adj;
    for (auto& kv : stable) if (kv.second) {
        NodeId a = kv.first.first, b = kv.first.second;
        if (R.is_core[a] && R.is_core[b]) { adj[a].push_back(b); adj[b].push_back(a); }
    }
    for (std::size_t s = 0; s < n; ++s) if (R.is_core[s] && R.community[s] == -1) {
        std::vector<int> st{(int)s}; R.community[s] = (int)s;
        while (!st.empty()) {
            int x = st.back(); st.pop_back();
            for (int y : adj[x]) if (R.community[y] == -1) { R.community[y] = (int)s; st.push_back(y); }
        }
    }
    for (auto& kv : stable) if (kv.second) {
        NodeId a = kv.first.first, b = kv.first.second;
        if (R.is_core[a] && !R.is_core[b] && R.community[b] == -1) R.community[b] = R.community[a];
        if (R.is_core[b] && !R.is_core[a] && R.community[a] == -1) R.community[a] = R.community[b];
    }
    return R;
}

} // namespace tkcore
