#pragma once
//
// loader.hpp
// Reader for SNAP-style temporal edge lists.
//
// Expected line format (whitespace separated):
//     <src> <dst> <timestamp>
// Lines beginning with '#' or '%' are treated as comments and skipped.
// A line with only "<src> <dst>" is accepted with timestamp 0 (so that plain
// static edge lists can be loaded too).
//
// This matches the format used by the Stanford SNAP temporal datasets
// (e.g. CollegeMsg, email-Eu-core-temporal, sx-* , wiki-talk-temporal),
// which are "u v unix_timestamp" per line.
//
#include "tkcore/temporal_graph.hpp"
#include <string>

namespace tkcore {

// Load a temporal edge list from `path`. Throws std::runtime_error if the file
// cannot be opened. The returned graph is already finalized().
TemporalGraph load_temporal_edgelist(const std::string& path);

} // namespace tkcore
