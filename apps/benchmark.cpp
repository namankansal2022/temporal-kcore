//
// benchmark.cpp
// Command-line benchmark harness.
//
//   Usage: tkcore_benchmark <edgelist> [--validate]
//
// Reports wall-clock time for load / statistics / k-core, peak resident memory,
// and basic graph statistics. With --validate it also runs the reference
// algorithm and checks that every core number matches (accuracy check).
//
#include "tkcore/temporal_graph.hpp"
#include "tkcore/loader.hpp"
#include "tkcore/statistics.hpp"
#include "tkcore/kcore.hpp"

#include <chrono>
#include <cstdio>
#include <string>
#include <sys/resource.h>

using clk = std::chrono::steady_clock;

static double ms_since(clk::time_point t0) {
    return std::chrono::duration<double, std::milli>(clk::now() - t0).count();
}

// peak resident set size in MiB (Linux: ru_maxrss is in KiB)
static double peak_rss_mib() {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return static_cast<double>(ru.ru_maxrss) / 1024.0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <edgelist> [--validate]\n", argv[0]);
        return 1;
    }
    std::string path = argv[1];
    bool validate = (argc >= 3 && std::string(argv[2]) == "--validate");

    std::printf("dataset: %s\n", path.c_str());

    // ---- load -------------------------------------------------------------
    auto t0 = clk::now();
    tkcore::TemporalGraph g = tkcore::load_temporal_edgelist(path);
    double t_load = ms_since(t0);

    // ---- statistics -------------------------------------------------------
    t0 = clk::now();
    tkcore::GraphStats s = tkcore::compute_statistics(g);
    double t_stats = ms_since(t0);

    // ---- k-core (fast) ----------------------------------------------------
    t0 = clk::now();
    auto core = tkcore::temporal_core_numbers(g);
    double t_core = ms_since(t0);

    tkcore::CoreNumber max_core = 0;
    for (auto c : core) if (c > max_core) max_core = c;

    // ---- report -----------------------------------------------------------
    std::printf("\n--- statistics ---\n");
    std::printf("nodes                : %zu\n", s.num_nodes);
    std::printf("temporal edges       : %zu\n", s.num_temporal_edges);
    std::printf("static edges (u,v)   : %zu\n", s.num_static_edges);
    std::printf("distinct timestamps  : %zu\n", s.num_distinct_times);
    std::printf("time span            : %lld  [%lld .. %lld]\n",
                (long long)s.time_span, (long long)s.min_time, (long long)s.max_time);
    std::printf("avg temporal degree  : %.3f\n", s.avg_temporal_degree);
    std::printf("max temporal degree  : %zu\n", s.max_temporal_degree);
    std::printf("max core number      : %u\n", max_core);

    std::printf("\n--- timing (ms) ---\n");
    std::printf("load                 : %.2f\n", t_load);
    std::printf("statistics           : %.2f\n", t_stats);
    std::printf("k-core (fast)        : %.2f\n", t_core);

    if (validate) {
        t0 = clk::now();
        auto ref = tkcore::temporal_core_numbers_reference(g);
        double t_ref = ms_since(t0);
        std::size_t mismatches = 0;
        for (std::size_t v = 0; v < core.size(); ++v)
            if (core[v] != ref[v]) ++mismatches;
        std::printf("k-core (reference)   : %.2f\n", t_ref);
        std::printf("\n--- validation ---\n");
        std::printf("mismatches           : %zu / %zu  -> %s\n",
                    mismatches, core.size(), mismatches == 0 ? "PASS" : "FAIL");
    }

    std::printf("\n--- memory ---\n");
    std::printf("peak RSS             : %.1f MiB\n", peak_rss_mib());
    return 0;
}
