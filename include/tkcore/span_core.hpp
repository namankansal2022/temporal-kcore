#pragma once
//
// span_core.hpp
// (k, Delta)-span-core  (Galimberti et al., TKDD 2021).
//
// Snapshots are the distinct timestamps, indexed 0..T-1 in sorted order. For a
// span Delta = [a, b] (inclusive snapshot indices), the "intersection graph"
// keeps only pairs (u, v) that interact in EVERY snapshot a..b. The
// (k, Delta)-span-core is the ordinary k-core of that intersection graph.
//
#include "tkcore/temporal_graph.hpp"
#include "tkcore/filtered_core.hpp"   // CoreNum
#include <vector>

namespace tkcore {

int num_snapshots(const TemporalGraph& g);
std::vector<CoreNum> span_core_numbers(const TemporalGraph& g, int a, int b);
std::vector<CoreNum> span_core_numbers_reference(const TemporalGraph& g, int a, int b);

} // namespace tkcore
