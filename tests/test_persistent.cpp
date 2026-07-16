#include "tkcore/temporal_graph.hpp"
#include "tkcore/persistent_core.hpp"
#include <cstdio>
#include <random>
#include <vector>
using namespace tkcore;
static int failures=0;
#define CHECK(c,m) do{ if(!(c)){ std::printf("FAIL: %s\n",m); ++failures; } }while(0)

static void test_paper_value(){
    TemporalGraph g;
    for(int t=1;t<=5;++t){ g.add_edge(1,2,t); g.add_edge(2,3,t); g.add_edge(1,3,t); }
    g.finalize();
    auto p=persistence_values(g,3,2);
    auto px=[&](long long ext){ for(NodeId v=0;v<g.num_nodes();++v) if(g.external_id(v)==ext) return p[v]; return -1LL; };
    CHECK(px(1)==4 && px(2)==4 && px(3)==4, "persistence F=4 matches paper example");
    CHECK(persistent_core(g,3,2,4).size()==3, "(3,4)-persistent 2-core has 3 nodes");
    CHECK(persistent_core(g,3,2,5).empty(), "(3,5)-persistent 2-core empty");
}

static void test_fuzz(){
    std::mt19937 rng(9182); int trials=1500, bad=0;
    for(int tr=0;tr<trials;++tr){
        int n=3+rng()%5, T=1+rng()%6, m=rng()%40;
        TemporalGraph g; g.add_edge(0,1,0);
        for(int i=0;i<m;++i){ long long u=rng()%n,v=rng()%n; if(u==v)continue; g.add_edge(u,v,(long long)(rng()%T)); }
        g.finalize();
        int theta=rng()%4, k=1+rng()%3;
        if(persistence_values(g,theta,k)!=persistence_values_reference(g,theta,k)) ++bad;
    }
    CHECK(bad==0,"persistence fast vs reference fuzz");
    std::printf("persistent fuzz: %d/%d matched\n", trials-bad, trials);
}

int main(){
    test_paper_value();
    test_fuzz();
    if(failures==0){ std::printf("\nALL PERSISTENT-CORE TESTS PASSED\n"); return 0; }
    std::printf("\n%d CHECK(S) FAILED\n",failures); return 1;
}
