#include "tkcore/temporal_graph.hpp"
#include "tkcore/dense_core.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

using namespace tkcore;

static int failures = 0;
#define CHECK(c,msg) do{ if(!(c)){ std::printf("FAIL: %s\n", msg); ++failures; } }while(0)

static std::vector<NodeId> srt(std::vector<NodeId> v){ std::sort(v.begin(),v.end()); return v; }

static void test_msd_examples() {
    std::vector<int> ex1 = {4,2,3,4,4,2};
    std::vector<int> ex2 = {4,2,3,4,4,2,2,6,1};
    CHECK(std::fabs(max_l_segment_density(ex1,3) - 11.0/3.0) < 1e-9, "MSD ex1");
    CHECK(std::fabs(max_l_segment_density(ex2,4) - 3.6)      < 1e-9, "MSD ex2");
}

static void test_msd_fuzz() {
    std::mt19937 rng(2024);
    int trials = 100000, bad = 0;
    for (int t=0;t<trials;++t){
        int n = 1 + rng()%14, l = 1 + rng()%n;
        std::vector<int> a(n); for(auto& x:a) x = rng()%8;
        if (std::fabs(max_l_segment_density(a,l) - max_l_segment_density_naive(a,l)) > 1e-9) ++bad;
    }
    CHECK(bad==0, "MSD fast vs naive fuzz");
    std::printf("MSD fuzz: %d/%d matched\n", trials-bad, trials);
}

static void test_dense_core_small() {
    TemporalGraph g;
    int quad[][2] = {{1,2},{1,3},{1,4},{2,3},{2,4},{3,4}};
    for (int t=1;t<=3;++t) for (auto& e: quad) g.add_edge(e[0], e[1], t);
    g.add_edge(1,5,1);
    g.finalize();
    auto core = srt(dense_core(g, 3, 3.0));
    auto ref  = srt(dense_core_reference(g, 3, 3.0));
    CHECK(core == ref, "dense_core matches reference on small example");
    bool has5 = false;
    for (NodeId v : core) if (g.external_id(v) == 5) has5 = true;
    CHECK(!has5, "loosely-attached node peeled from dense core");
}

static void test_dense_core_fuzz() {
    std::mt19937 rng(77);
    int trials = 1500, bad = 0;
    for (int trial=0; trial<trials; ++trial){
        int n = 3 + rng()%7;
        int T = 2 + rng()%6;
        int m = rng()%40;
        TemporalGraph g;
        for (int i=0;i<m;++i){
            long long u = rng()%n, v = rng()%n; if (u==v) continue;
            long long t = 1 + rng()%T;
            g.add_edge(u,v,t);
        }
        g.finalize();
        int l = 1 + rng()%T;
        double delta = 1.0 + (rng()%5);
        if (srt(dense_core(g,l,delta)) != srt(dense_core_reference(g,l,delta))) ++bad;
    }
    CHECK(bad==0, "dense_core fast vs reference fuzz");
    std::printf("dense_core fuzz: %d/%d matched\n", trials-bad, trials);
}

int main(){
    test_msd_examples();
    test_msd_fuzz();
    test_dense_core_small();
    test_dense_core_fuzz();
    if (failures==0){ std::printf("\nALL DENSE-CORE TESTS PASSED\n"); return 0; }
    std::printf("\n%d CHECK(S) FAILED\n", failures);
    return 1;
}
