#include "tkcore/temporal_graph.hpp"
#include "tkcore/span_core.hpp"
#include <cstdio>
#include <random>
#include <vector>
using namespace tkcore;
static int failures=0;
#define CHECK(c,m) do{ if(!(c)){ std::printf("FAIL: %s\n",m); ++failures; } }while(0)

static void test_example(){
    TemporalGraph g;
    for(int t=0;t<3;++t){ g.add_edge(1,2,t); g.add_edge(2,3,t); g.add_edge(1,3,t); }
    g.add_edge(3,4,0);
    g.finalize();
    CHECK(num_snapshots(g)==3, "3 snapshots");
    auto s = span_core_numbers(g,0,2);
    auto cx=[&](long long ext){ for(NodeId v=0;v<g.num_nodes();++v) if(g.external_id(v)==ext) return (int)s[v]; return -1; };
    CHECK(cx(1)==2&&cx(2)==2&&cx(3)==2, "triangle span-core order 2 over [0,2]");
    CHECK(cx(4)==0, "d not in span-core (edge only in snapshot 0)");
}

static void test_fuzz(){
    std::mt19937 rng(808); int trials=3000, bad=0;
    for(int tr=0;tr<trials;++tr){
        int n=3+rng()%6, T=1+rng()%5, m=rng()%40;
        TemporalGraph g; g.add_edge(0,1,0);
        for(int i=0;i<m;++i){ long long u=rng()%n,v=rng()%n; if(u==v)continue; g.add_edge(u,v,(long long)(rng()%T)); }
        g.finalize();
        int TT=num_snapshots(g); int a=rng()%TT, b=a+rng()%(TT-a);
        if(span_core_numbers(g,a,b)!=span_core_numbers_reference(g,a,b)) ++bad;
    }
    CHECK(bad==0,"span-core fast vs reference fuzz");
    std::printf("span-core fuzz: %d/%d matched\n", trials-bad, trials);
}

int main(){
    test_example();
    test_fuzz();
    if(failures==0){ std::printf("\nALL SPAN-CORE TESTS PASSED\n"); return 0; }
    std::printf("\n%d CHECK(S) FAILED\n",failures); return 1;
}
