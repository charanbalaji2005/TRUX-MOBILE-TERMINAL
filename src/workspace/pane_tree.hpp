#pragma once
// meridian-terminal / workspace / pane_tree.hpp
//
// Binary Space Partitioning (BSP) tree for terminal pane multiplexing.
// Supports drag-to-resize, pane zoom, swap, directional navigation,
// pinned/floating panes, and synchronized input broadcasting.

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace meridian::workspace {

enum class SplitDirection {
    Horizontal, // Split top / bottom
    Vertical    // Split left / right
};

enum class NavigationDirection {
    Up,
    Down,
    Left,
    Right
};

struct PaneRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct PaneLayoutItem {
    uint32_t pane_id = 0;
    std::string title;
    std::string cwd;
    PaneRect bounds;
    bool is_focused = false;
    bool is_pinned = false;
    bool is_floating = false;
};

struct PaneNode {
    uint32_t pane_id = 0; // 0 for internal split nodes
    std::string title = "Terminal";
    std::string cwd;
    bool is_pinned = false;
    bool is_floating = false;

    // Split properties (if internal node)
    SplitDirection split_direction = SplitDirection::Vertical;
    float split_ratio = 0.5f; // [0.05, 0.95]
    std::unique_ptr<PaneNode> first_child;  // Left or Top
    std::unique_ptr<PaneNode> second_child; // Right or Bottom

    bool is_leaf() const { return first_child == nullptr && second_child == nullptr; }
};

class PaneTree {
public:
    PaneTree();
    ~PaneTree() = default;

    uint32_t active_pane_id() const { return active_pane_id_; }
    void set_active_pane(uint32_t id) { active_pane_id_ = id; }

    uint32_t split_pane(uint32_t target_pane_id, SplitDirection direction, float ratio = 0.5f, const std::string& cwd = "");
    bool close_pane(uint32_t pane_id);
    bool resize_pane(uint32_t pane_id, float delta_ratio);
    bool swap_panes(uint32_t pane_a, uint32_t pane_b);

    // Zooming (maximizing active pane)
    void toggle_zoom();
    bool is_zoomed() const { return zoomed_pane_id_.has_value(); }

    // Synchronized input across panes
    bool sync_input() const { return sync_input_; }
    void set_sync_input(bool enabled) { sync_input_ = enabled; }

    // Directional navigation
    std::optional<uint32_t> find_adjacent_pane(uint32_t from_id, NavigationDirection dir, int total_w = 80, int total_h = 24) const;

    // Pinning
    void set_pane_pinned(uint32_t pane_id, bool pinned);

    // Compute screen geometries
    std::vector<PaneLayoutItem> compute_layout(int total_width, int total_height) const;

    std::size_t count_panes() const;
    std::vector<uint32_t> all_pane_ids() const;

    const PaneNode* root() const { return root_.get(); }
    void set_root(std::unique_ptr<PaneNode> new_root, uint32_t next_id, uint32_t active_id);

private:
    uint32_t next_pane_id_ = 1;
    uint32_t active_pane_id_ = 1;
    std::optional<uint32_t> zoomed_pane_id_;
    bool sync_input_ = false;
    std::unique_ptr<PaneNode> root_;

    PaneNode* find_node(PaneNode* current, uint32_t id);
    const PaneNode* find_node(const PaneNode* current, uint32_t id) const;
    PaneNode* find_parent(PaneNode* current, uint32_t target_id);
    void collect_panes(const PaneNode* node, std::vector<uint32_t>& out) const;
    void layout_recursive(const PaneNode* node, const PaneRect& rect, std::vector<PaneLayoutItem>& out) const;
};

// Global session pane tree instance shared across the interactive shell and builtins
PaneTree& get_session_pane_tree();

} // namespace meridian::workspace

