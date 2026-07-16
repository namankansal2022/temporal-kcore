#include "tkcore/temporal_graph.hpp"
#include "tkcore/stable_core.hpp"
#include <algorithm>
#include <cstdio>
#include <map>
#include <random>
#include <set>
#include <vector>
using namespace tkcore;
static int failures=0;
#define CHECK(c,m) do{ if(!(c)){ std::printf("FAIL: %s\n",m); ++failures; } }while(0)

static std::vector<std::set<int>> parts(const std::vector<int>& comm){
    std::map<int,std::set<int>> mp;
    for(int i=0;i<(int)comm.size();++i) if(comm[i]!=-1) mp[comm[i]].insert(i);
    std::vector<std::set<int>> out; for(auto&kv:mp) out.push_back(kv.second);
    std::sort(out.begin(),out.end()); return out;
}

static void test_triangle_over_time(){
    TemporalGraph g;
    for(int t=1;t<=3;++t){ g.add_edge(1,2,t); g.add_edge(2,3,t); g.add_edge(1,3,t); }
    g.finalize();
    auto R = stable_cores(g, 2, 3, 0.5);
    int cores=0; for(char c:R.is_core) cores+=c;
    CHECK(cores==3, "triangle: all 3 nodes are stable cores");
    auto Rr = stable_cores_reference(g,2,3,0.5);
    CHECK(R.is_core==Rr.is_core && parts(R.community)==parts(Rr.community), "triangle matches reference");
}

static void test_fuzz(){
    std::mt19937 rng(321); int trials=4000, badc=0, badk=0;
    for(int tr=0;tr<trials;++tr){
        int n=3+rng()%7, T=1+rng()%4, m=rng()%30;
        TemporalGraph g; bool any=false;
        for(int i=0;i<m;++i){ long long u=rng()%n,v=rng()%n; if(u==v)continue; g.add_edge(u,v,(long long)(rng()%T)); any=true; }
        if(!any) g.add_edge(0,1,0);
        g.finalize();
        int mu=1+rng()%3, tau=1+rng()%2; double eps=0.2+0.2*(rng()%4);
        auto A=stable_cores(g,mu,tau,eps); auto B=stable_cores_reference(g,mu,tau,eps);
        if(A.is_core!=B.is_core) ++badk;
        if(parts(A.community)!=parts(B.community)) ++badc;
    }
    CHECK(badk==0,"fuzz: core sets match reference");
    CHECK(badc==0,"fuzz: community partitions match reference");
    std::printf("stable fuzz: cores %d/%d, clusters %d/%d\n", trials-badk,trials, trials-badc,trials);
}

int main(){
    test_triangle_over_time();
    test_fuzz();
    if(failures==0){ std::printf("\nALL STABLE-CORE TESTS PASSED\n"); return 0; }
    std::printf("\n%d CHECK(S) FAILED\n",failures); return 1;
}
