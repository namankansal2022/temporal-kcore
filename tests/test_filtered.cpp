#include "tkcore/temporal_graph.hpp"
#include "tkcore/filtered_core.hpp"
#include <cstdio>
#include <random>
#include <vector>
#include <set>
using namespace tkcore;
static int failures=0;
#define CHECK(c,m) do{ if(!(c)){ std::printf("FAIL: %s\n",m); ++failures; } }while(0)

static void test_engine_fuzz(){
    std::mt19937 rng(555); int trials=4000, bad=0;
    for(int tr=0;tr<trials;++tr){
        int n=1+rng()%12;
        std::vector<std::set<NodeId>> S(n);
        int m=rng()%30;
        for(int i=0;i<m;++i){ NodeId u=rng()%n,v=rng()%n; if(u==v)continue; S[u].insert(v); S[v].insert(u); }
        std::vector<std::vector<NodeId>> adj(n);
        for(int i=0;i<n;++i) adj[i].assign(S[i].begin(),S[i].end());
        if(simple_core_numbers(adj)!=simple_core_numbers_reference(adj)) ++bad;
    }
    CHECK(bad==0,"simple k-core fast vs reference fuzz");
    std::printf("simple k-core fuzz: %d/%d matched\n", trials-bad, trials);
}

static void test_kh(){
    TemporalGraph g;
    g.add_edge(1,2,10); g.add_edge(1,2,11); g.add_edge(1,2,12);
    g.add_edge(2,3,10); g.add_edge(1,3,10);
    g.finalize();
    auto h1 = kh_core_numbers(g,1);
    auto h3 = kh_core_numbers(g,3);
    auto cx=[&](const std::vector<CoreNum>&c,long long ext){ for(NodeId v=0;v<g.num_nodes();++v) if(g.external_id(v)==ext) return (int)c[v]; return -1; };
    CHECK(cx(h1,1)==2 && cx(h1,2)==2 && cx(h1,3)==2, "(k,h=1) triangle -> core 2");
    CHECK(cx(h3,1)==1 && cx(h3,2)==1 && cx(h3,3)==0, "(k,h=3) only heavy edge kept");
}

static void test_window(){
    TemporalGraph g;
    g.add_edge(1,2,5); g.add_edge(2,3,5); g.add_edge(1,3,5);
    g.add_edge(3,4,100);
    g.finalize();
    auto w = window_core_numbers(g,0,10);
    auto cx=[&](long long ext){ for(NodeId v=0;v<g.num_nodes();++v) if(g.external_id(v)==ext) return (int)w[v]; return -1; };
    CHECK(cx(1)==2 && cx(2)==2 && cx(3)==2, "window triangle -> core 2");
    CHECK(cx(4)==0, "node outside window has core 0");
}

int main(){
    test_engine_fuzz();
    test_kh();
    test_window();
    if(failures==0){ std::printf("\nALL FILTERED-CORE TESTS PASSED\n"); return 0; }
    std::printf("\n%d CHECK(S) FAILED\n",failures); return 1;
}
