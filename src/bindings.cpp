//
// bindings.cpp
// Python bindings for the tkcore library via pybind11.
//
// Exposes: TemporalGraph, load, statistics, and the two core-number routines.
// Node ids returned to Python are ORIGINAL dataset ids, and core numbers are
// returned as a dict { original_node_id : core_number } for convenience.
//
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "tkcore/temporal_graph.hpp"
#include "tkcore/loader.hpp"
#include "tkcore/statistics.hpp"
#include "tkcore/kcore.hpp"
#include "tkcore/dense_core.hpp"
#include "tkcore/stable_core.hpp"

#include <map>
#include <vector>

namespace py = pybind11;
using namespace tkcore;

static std::map<std::int64_t, CoreNumber>
core_numbers_by_external_id(const TemporalGraph& g, bool reference) {
    auto core = reference ? temporal_core_numbers_reference(g)
                          : temporal_core_numbers(g);
    std::map<std::int64_t, CoreNumber> out;
    for (NodeId v = 0; v < g.num_nodes(); ++v)
        out[g.external_id(v)] = core[v];
    return out;
}

PYBIND11_MODULE(pytkcore, m) {
    m.doc() = "Temporal k-core decomposition (C++ core with pybind11 bindings)";

    py::class_<TemporalGraph>(m, "TemporalGraph")
        .def(py::init<>())
        .def("add_edge", &TemporalGraph::add_edge,
             py::arg("src"), py::arg("dst"), py::arg("t"))
        .def("finalize", &TemporalGraph::finalize)
        .def("num_nodes", &TemporalGraph::num_nodes)
        .def("num_edges", &TemporalGraph::num_edges)
        .def("num_timestamps", &TemporalGraph::num_timestamps)
        .def("min_time", &TemporalGraph::min_time)
        .def("max_time", &TemporalGraph::max_time)
        .def("__repr__", [](const TemporalGraph& g) {
            return "<TemporalGraph nodes=" + std::to_string(g.num_nodes()) +
                   " edges=" + std::to_string(g.num_edges()) +
                   " timestamps=" + std::to_string(g.num_timestamps()) + ">";
        });

    py::class_<GraphStats>(m, "GraphStats")
        .def_readonly("num_nodes",           &GraphStats::num_nodes)
        .def_readonly("num_temporal_edges",  &GraphStats::num_temporal_edges)
        .def_readonly("num_static_edges",    &GraphStats::num_static_edges)
        .def_readonly("num_distinct_times",  &GraphStats::num_distinct_times)
        .def_readonly("min_time",            &GraphStats::min_time)
        .def_readonly("max_time",            &GraphStats::max_time)
        .def_readonly("time_span",           &GraphStats::time_span)
        .def_readonly("avg_temporal_degree", &GraphStats::avg_temporal_degree)
        .def_readonly("max_temporal_degree", &GraphStats::max_temporal_degree)
        .def("__repr__", [](const GraphStats& s) {
            return "<GraphStats nodes=" + std::to_string(s.num_nodes) +
                   " temporal_edges=" + std::to_string(s.num_temporal_edges) +
                   " static_edges=" + std::to_string(s.num_static_edges) +
                   " timestamps=" + std::to_string(s.num_distinct_times) + ">";
        });

    m.def("load", &load_temporal_edgelist, py::arg("path"),
          "Load a SNAP-style temporal edge list ('src dst timestamp' per line).");

    m.def("statistics", &compute_statistics, py::arg("graph"),
          "Compute basic statistics of a temporal graph.");

    m.def("core_numbers",
          [](const TemporalGraph& g) { return core_numbers_by_external_id(g, false); },
          py::arg("graph"),
          "Temporal-degree k-core numbers { node_id : core } (fast linear-time).");

    m.def("core_numbers_reference",
          [](const TemporalGraph& g) { return core_numbers_by_external_id(g, true); },
          py::arg("graph"),
          "Reference core numbers (slow, for validation).");

    m.def("dense_core",
          [](const TemporalGraph& g, int l, double delta) {
              auto ids = dense_core(g, l, delta);
              std::vector<std::int64_t> out; out.reserve(ids.size());
              for (NodeId v : ids) out.push_back(g.external_id(v));
              return out;
          },
          py::arg("graph"), py::arg("l"), py::arg("delta"),
          "(l, delta)-maximal dense core: node ids in the bursting core.");

    m.def("stable_core_nodes",
          [](const TemporalGraph& g, int mu, int tau, double eps) {
              auto R = stable_cores(g, mu, tau, eps);
              std::vector<std::int64_t> out;
              for (NodeId v = 0; v < g.num_nodes(); ++v)
                  if (R.is_core[v]) out.push_back(g.external_id(v));
              return out;
          },
          py::arg("graph"), py::arg("mu"), py::arg("tau"), py::arg("eps"),
          "(mu,tau,eps)-stable core node ids.");

    m.def("stable_communities",
          [](const TemporalGraph& g, int mu, int tau, double eps) {
              auto R = stable_cores(g, mu, tau, eps);
              std::map<std::int64_t, std::int64_t> out;
              for (NodeId v = 0; v < g.num_nodes(); ++v)
                  if (R.community[v] != -1)
                      out[g.external_id(v)] = g.external_id((NodeId)R.community[v]);
              return out;
          },
          py::arg("graph"), py::arg("mu"), py::arg("tau"), py::arg("eps"),
          "(mu,tau,eps)-stable communities: {node: community representative}.");
}
