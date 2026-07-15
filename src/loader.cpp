#include "tkcore/loader.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace tkcore {

// A small, fast buffered reader. We avoid iostream for the hot path because
// temporal datasets can have tens of millions of lines; std::strtoll over a
// raw buffer is considerably faster than operator>> on std::ifstream.
TemporalGraph load_temporal_edgelist(const std::string& path) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) throw std::runtime_error("could not open file: " + path);

    TemporalGraph g;

    constexpr std::size_t BUF = 1 << 20; // 1 MiB read buffer
    std::vector<char> buffer(BUF);
    std::string line;
    line.reserve(64);

    auto process_line = [&](const std::string& s) {
        // skip leading whitespace
        std::size_t i = 0;
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r')) ++i;
        if (i >= s.size()) return;                 // empty
        if (s[i] == '#' || s[i] == '%') return;    // comment

        const char* p = s.c_str() + i;
        char* end = nullptr;

        long long src = std::strtoll(p, &end, 10);
        if (end == p) return;                      // no number -> skip
        p = end;
        long long dst = std::strtoll(p, &end, 10);
        if (end == p) return;                      // need at least two columns
        p = end;
        long long ts = std::strtoll(p, &end, 10);  // optional third column
        if (end == p) ts = 0;

        g.add_edge(src, dst, static_cast<Timestamp>(ts));
    };

    std::size_t nread;
    while ((nread = std::fread(buffer.data(), 1, BUF, f)) > 0) {
        for (std::size_t k = 0; k < nread; ++k) {
            char c = buffer[k];
            if (c == '\n') {
                process_line(line);
                line.clear();
            } else {
                line.push_back(c);
            }
        }
    }
    if (!line.empty()) process_line(line);         // last line without newline

    std::fclose(f);
    g.finalize();
    return g;
}

} // namespace tkcore
