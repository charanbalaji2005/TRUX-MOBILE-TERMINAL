#pragma once
// meridian-terminal / dev / universal_search.hpp
//
// Universal multi-target search engine (Ctrl+Shift+F). Searches across
// active screen buffers, scrollback history, command execution history, and files.

#include "../core/vt/screen_buffer.hpp"
#include "rich_history.hpp"

#include <string>
#include <vector>

namespace meridian::dev {

enum class SearchSource {
    ActiveScreen,
    Scrollback,
    CommandHistory,
    ProjectFile
};

struct SearchMatch {
    SearchSource source = SearchSource::ActiveScreen;
    std::string source_label;
    int line_number = 0;
    std::string line_content;
    std::size_t match_offset = 0;
};

class UniversalSearch {
public:
    UniversalSearch() = default;

    static std::vector<SearchMatch> search_screen(const vt::ScreenBuffer& screen, const std::string& query);
    static std::vector<SearchMatch> search_history(const RichHistory& history, const std::string& query);
    static std::vector<SearchMatch> search_all(
        const vt::ScreenBuffer& screen,
        const RichHistory& history,
        const std::string& query
    );
};

} // namespace meridian::dev

