#include "universal_search.hpp"

namespace meridian::dev {

std::vector<SearchMatch> UniversalSearch::search_screen(const vt::ScreenBuffer& screen, const std::string& query) {
    std::vector<SearchMatch> matches;
    if (query.empty()) return matches;

    int rows = screen.rows();
    for (int r = 0; r < rows; ++r) {
        std::string row_text = screen.dump_row_text(r);
        auto pos = row_text.find(query);
        if (pos != std::string::npos) {
            SearchMatch m;
            m.source = SearchSource::ActiveScreen;
            m.source_label = "Screen (row " + std::to_string(r + 1) + ")";
            m.line_number = r + 1;
            m.line_content = row_text;
            m.match_offset = pos;
            matches.push_back(m);
        }
    }
    return matches;
}

std::vector<SearchMatch> UniversalSearch::search_history(const RichHistory& history, const std::string& query) {
    std::vector<SearchMatch> matches;
    if (query.empty()) return matches;

    auto records = history.search(query, 50);
    for (const auto& rec : records) {
        SearchMatch m;
        m.source = SearchSource::CommandHistory;
        m.source_label = "History (" + rec.working_dir + ")";
        m.line_number = static_cast<int>(rec.id);
        m.line_content = rec.command;
        m.match_offset = rec.command.find(query);
        matches.push_back(m);
    }
    return matches;
}

std::vector<SearchMatch> UniversalSearch::search_all(
    const vt::ScreenBuffer& screen,
    const RichHistory& history,
    const std::string& query
) {
    auto screen_matches = search_screen(screen, query);
    auto history_matches = search_history(history, query);

    std::vector<SearchMatch> combined;
    combined.insert(combined.end(), screen_matches.begin(), screen_matches.end());
    combined.insert(combined.end(), history_matches.begin(), history_matches.end());
    return combined;
}

} // namespace meridian::dev

