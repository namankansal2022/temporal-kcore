#include "tkcore/temporal_graph.hpp"
#include "tkcore/pseudocore.hpp"
#include <cstdio>
#include <random>
#include <vector>
#include <map>
using namespace tkcore;
static int failures=0;
#define CHECK(c,m) do{ if(!(c)){ std::printf("FAIL: %s\n",m); ++failures; } }while(0)

static void test_h0_is_temporal_degree(){
    TemporalGraph g;
    g.add_edge(1,2,5); g.add_edge(1,2,6); g.add_edge(2,3,7); g.add_edge(1,3,8);
    g.finalize();
    auto h0 = temporal_h_index(g,0);
    auto cx=[&](long long ext){ for(NodeId v=0;v<g.num_nodes();++v) if(g.external_id(v)==ext) return (int)h0[v]; return -1; };
    CHECK(cx(1)==3, "h0(1) = 3 incident temporal edges");
    CHECK(cx(2)==3, "h0(2) = 3");
    CHECK(cx(3)==2, "h0(3) = 2");
}

static void test_pseudocore_monotone(){
    TemporalGraph g;
    for(int t=1;t<=4;++t){ g.add_edge(1,2,t); g.add_edge(2,3,t); g.add_edge(1,3,t); g.add_edge(3,4,t);}
    g.finalize();
    auto p1 = pseudocore(g,2,1);
    auto p3 = pseudocore(g,2,3);
    CHECK(p1.size() >= p3.size(), "pseudocore shrinks (or equal) as k grows");
}

static void test_fuzz(){
    std::mt19937 rng(4242); int trials=3000, bad=0;
    for(int tr=0;tr<trials;++tr){
        int n=2+rng()%4, T=2+rng()%2, m=rng()%12;
        TemporalGraph g; g.add_edge(0,1,1);
        for(int i=0;i<m;++i){ long long u=rng()%n,v=rng()%n; if(u==v)continue; g.add_edge(u,v,(long long)(1+rng()%T)); }
        g.finalize();
        int ord = rng()%4;
        if(temporal_h_index(g,ord)!=temporal_h_index_reference(g,ord)) ++bad;
    }
    CHECK(bad==0,"temporal H-index memoised vs tree-expansion fuzz");
    std::printf("pseudocore fuzz: %d/%d matched\n", trials-bad, trials);
}

int main(){
    test_h0_is_temporal_degree();
    test_pseudocore_monotone();
    test_fuzz();
    if(failures==0){ std::printf("\nALL PSEUDOCORE TESTS PASSED\n"); return 0; }
    std::printf("\n%d CHECK(S) FAILED\n",failures); return 1;
}
