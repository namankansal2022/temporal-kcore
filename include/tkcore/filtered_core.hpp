#pragma once
//
// filtered_core.hpp
// The peeling family: static k-core, (k,h)-core, and time-window / historical
// k-core -- the same simple-graph k-core peeling on a different filtered edge
// set (all pairs / multiplicity >= h / pairs interacting within [ts, te]).
// Degree = number of DISTINCT qualifying neighbours (distinguishing these from
// the temporal-degree k-core).
//
#include "tkcore/temporal_graph.hpp"
#include <cstdint>
#include <vector>

namespace tkcore {

using CoreNum = std::uint32_t;

std::vector<CoreNum> simple_core_numbers(const std::vector<std::vector<NodeId>>& adj);
std::vector<CoreNum> simple_core_numbers_reference(const std::vector<std::vector<NodeId>>& adj);

std::vector<CoreNum> static_core_numbers(const TemporalGraph& g);
std::vector<CoreNum> kh_core_numbers(const TemporalGraph& g, int h);
std::vector<CoreNum> window_core_numbers(const TemporalGraph& g, Timestamp ts, Timestamp te);

} // namespace tkcore
