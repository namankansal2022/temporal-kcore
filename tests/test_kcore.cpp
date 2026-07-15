//
// test_kcore.cpp
// Correctness tests for the temporal-degree k-core.
//   1. a hand-checked worked example
//   2. randomized fuzzing: fast algorithm vs. independent reference
// Exit code 0 = all pass, 1 = failure.
//
#include "tkcore/temporal_graph.hpp"
#include "tkcore/kcore.hpp"

#include <array>
#include <cstdio>
#include <map>
#include <random>
#include <vector>

using namespace tkcore;

static int failures = 0;
#define CHECK(cond, msg) do { if(!(cond)){ std::printf("FAIL: %s\n", msg); ++failures; } } while(0)

// Build a graph from (u,v,t) triples on ORIGINAL ids and return core by ext id.
static std::map<std::int64_t, CoreNumber>
cores(const std::vector<std::array<long long,3>>& E, bool ref=false) {
    TemporalGraph g;
    for (auto& e : E) g.add_edge(e[0], e[1], e[2]);
    g.finalize();
    auto c = ref ? temporal_core_numbers_reference(g) : temporal_core_numbers(g);
    std::map<std::int64_t, CoreNumber> out;
    for (NodeId v = 0; v < g.num_nodes(); ++v) out[g.external_id(v)] = c[v];
    return out;
}

static void test_worked_example() {
    // Triangle a-b-c (one temporal edge each) plus a pendant d hanging off a.
    // Temporal degrees: a=3, b=2, c=2, d=1.
    //   d is peeled first  -> core(d)=1, a drops to 2
    //   then a,b,c form a triangle, each degree 2 -> core = 2
    std::vector<std::array<long long,3>> E = {
        {1,2,10},{2,3,11},{3,1,12},{1,4,13}
    };
    auto c = cores(E);
    CHECK(c[4]==1, "worked: d core 1");
    CHECK(c[1]==2, "worked: a core 2");
    CHECK(c[2]==2, "worked: b core 2");
    CHECK(c[3]==2, "worked: c core 2");
}

static void test_multiplicity() {
    // Two nodes joined by 3 parallel temporal edges => each has temporal
    // degree 3 => both are in the 3-core.
    std::vector<std::array<long long,3>> E = {
        {1,2,1},{1,2,2},{1,2,3}
    };
    auto c = cores(E);
    CHECK(c[1]==3 && c[2]==3, "multiplicity: parallel edges counted");
}

static void test_fuzz() {
    std::mt19937 rng(12345);
    int trials = 2000, bad = 0;
    for (int trial = 0; trial < trials; ++trial) {
        int n = 2 + rng() % 12;                 // 2..13 nodes
        int m = rng() % 40;                     // 0..39 temporal edges
        std::vector<std::array<long long,3>> E;
        for (int i = 0; i < m; ++i) {
            long long u = rng() % n;
            long long v = rng() % n;
            if (u == v) continue;               // ignore self loops
            long long t = rng() % 20;
            E.push_back({u, v, t});
        }
        auto fast = cores(E, false);
        auto ref  = cores(E, true);
        if (fast != ref) { ++bad; if (bad <= 3) std::printf("  fuzz mismatch trial %d\n", trial); }
    }
    CHECK(bad == 0, "fuzz: fast matches reference on all random multigraphs");
    std::printf("fuzz: %d/%d trials matched\n", trials - bad, trials);
}

int main() {
    test_worked_example();
    test_multiplicity();
    test_fuzz();
    if (failures == 0) { std::printf("\nALL TESTS PASSED\n"); return 0; }
    std::printf("\n%d CHECK(S) FAILED\n", failures);
    return 1;
}
