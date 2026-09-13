#include "pane_tree.hpp"

#include <algorithm>

namespace meridian::workspace {

PaneTree::PaneTree() {
    root_ = std::make_unique<PaneNode>();
    root_->pane_id = next_pane_id_++;
    active_pane_id_ = root_->pane_id;
}

void PaneTree::set_root(std::unique_ptr<PaneNode> new_root, uint32_t next_id, uint32_t active_id) {
    root_ = std::move(new_root);
    next_pane_id_ = next_id;
    active_pane_id_ = active_id;
}

PaneNode* PaneTree::find_node(PaneNode* current, uint32_t id) {
    if (!current) return nullptr;
    if (current->is_leaf()) {
        return current->pane_id == id ? current : nullptr;
    }
    PaneNode* res = find_node(current->first_child.get(), id);
    if (res) return res;
    return find_node(current->second_child.get(), id);
}

const PaneNode* PaneTree::find_node(const PaneNode* current, uint32_t id) const {
    if (!current) return nullptr;
    if (current->is_leaf()) {
        return current->pane_id == id ? current : nullptr;
    }
    const PaneNode* res = find_node(current->first_child.get(), id);
    if (res) return res;
    return find_node(current->second_child.get(), id);
}

PaneNode* PaneTree::find_parent(PaneNode* current, uint32_t target_id) {
    if (!current || current->is_leaf()) return nullptr;
    if ((current->first_child && current->first_child->pane_id == target_id) ||
        (current->second_child && current->second_child->pane_id == target_id)) {
        return current;
    }
    PaneNode* res = find_parent(current->first_child.get(), target_id);
    if (res) return res;
    return find_parent(current->second_child.get(), target_id);
}

uint32_t PaneTree::split_pane(uint32_t target_pane_id, SplitDirection direction, float ratio, const std::string& cwd) {
    PaneNode* target = find_node(root_.get(), target_pane_id);
    if (!target) return 0;

    // Convert target leaf to an internal split node
    uint32_t new_pane_id = next_pane_id_++;

    auto old_leaf = std::make_unique<PaneNode>();
    old_leaf->pane_id = target->pane_id;
    old_leaf->title = target->title;
    old_leaf->cwd = target->cwd;
    old_leaf->is_pinned = target->is_pinned;

    auto new_leaf = std::make_unique<PaneNode>();
    new_leaf->pane_id = new_pane_id;
    new_leaf->title = "Terminal";
    new_leaf->cwd = cwd.empty() ? target->cwd : cwd;

    target->pane_id = 0; // Mark as internal split
    target->split_direction = direction;
    target->split_ratio = std::clamp(ratio, 0.1f, 0.9f);
    target->first_child = std::move(old_leaf);
    target->second_child = std::move(new_leaf);

    active_pane_id_ = new_pane_id;
    return new_pane_id;
}

bool PaneTree::close_pane(uint32_t pane_id) {
    if (count_panes() <= 1) return false; // Don't close last pane

    PaneNode* parent = find_parent(root_.get(), pane_id);
    if (!parent) return false;

    // Sibling becomes the parent's content
    std::unique_ptr<PaneNode> sibling;
    if (parent->first_child && parent->first_child->pane_id == pane_id) {
        sibling = std::move(parent->second_child);
    } else {
        sibling = std::move(parent->first_child);
    }

    // Copy sibling's properties into parent
    parent->pane_id = sibling->pane_id;
    parent->title = sibling->title;
    parent->cwd = sibling->cwd;
    parent->is_pinned = sibling->is_pinned;
    parent->split_direction = sibling->split_direction;
    parent->split_ratio = sibling->split_ratio;
    parent->first_child = std::move(sibling->first_child);
    parent->second_child = std::move(sibling->second_child);

    if (active_pane_id_ == pane_id) {
        auto all = all_pane_ids();
        active_pane_id_ = all.empty() ? 0 : all.front();
    }
    if (zoomed_pane_id_ == pane_id) {
        zoomed_pane_id_.reset();
    }

    return true;
}

bool PaneTree::resize_pane(uint32_t pane_id, float delta_ratio) {
    PaneNode* parent = find_parent(root_.get(), pane_id);
    if (!parent) return false;

    if (parent->first_child && parent->first_child->pane_id == pane_id) {
        parent->split_ratio = std::clamp(parent->split_ratio + delta_ratio, 0.1f, 0.9f);
    } else {
        parent->split_ratio = std::clamp(parent->split_ratio - delta_ratio, 0.1f, 0.9f);
    }
    return true;
}

bool PaneTree::swap_panes(uint32_t pane_a, uint32_t pane_b) {
    PaneNode* node_a = find_node(root_.get(), pane_a);
    PaneNode* node_b = find_node(root_.get(), pane_b);
    if (!node_a || !node_b) return false;

    std::swap(node_a->title, node_b->title);
    std::swap(node_a->cwd, node_b->cwd);
    std::swap(node_a->is_pinned, node_b->is_pinned);
    std::swap(node_a->pane_id, node_b->pane_id);
    return true;
}

void PaneTree::toggle_zoom() {
    if (zoomed_pane_id_.has_value()) {
        zoomed_pane_id_.reset();
    } else {
        zoomed_pane_id_ = active_pane_id_;
    }
}

void PaneTree::set_pane_pinned(uint32_t pane_id, bool pinned) {
    PaneNode* node = find_node(root_.get(), pane_id);
    if (node) node->is_pinned = pinned;
}

void PaneTree::collect_panes(const PaneNode* node, std::vector<uint32_t>& out) const {
    if (!node) return;
    if (node->is_leaf()) {
        out.push_back(node->pane_id);
        return;
    }
    collect_panes(node->first_child.get(), out);
    collect_panes(node->second_child.get(), out);
}

std::vector<uint32_t> PaneTree::all_pane_ids() const {
    std::vector<uint32_t> ids;
    collect_panes(root_.get(), ids);
    return ids;
}

std::size_t PaneTree::count_panes() const {
    return all_pane_ids().size();
}

void PaneTree::layout_recursive(const PaneNode* node, const PaneRect& rect, std::vector<PaneLayoutItem>& out) const {
    if (!node) return;
    if (node->is_leaf()) {
        PaneLayoutItem item;
        item.pane_id = node->pane_id;
        item.title = node->title;
        item.cwd = node->cwd;
        item.bounds = rect;
        item.is_focused = (node->pane_id == active_pane_id_);
        item.is_pinned = node->is_pinned;
        item.is_floating = node->is_floating;
        out.push_back(item);
        return;
    }

    if (node->split_direction == SplitDirection::Vertical) {
        int w1 = static_cast<int>(rect.width * node->split_ratio);
        int w2 = rect.width - w1;
        PaneRect r1{rect.x, rect.y, w1, rect.height};
        PaneRect r2{rect.x + w1, rect.y, w2, rect.height};
        layout_recursive(node->first_child.get(), r1, out);
        layout_recursive(node->second_child.get(), r2, out);
    } else {
        int h1 = static_cast<int>(rect.height * node->split_ratio);
        int h2 = rect.height - h1;
        PaneRect r1{rect.x, rect.y, rect.width, h1};
        PaneRect r2{rect.x, rect.y + h1, rect.width, h2};
        layout_recursive(node->first_child.get(), r1, out);
        layout_recursive(node->second_child.get(), r2, out);
    }
}

std::vector<PaneLayoutItem> PaneTree::compute_layout(int total_width, int total_height) const {
    std::vector<PaneLayoutItem> items;
    if (zoomed_pane_id_.has_value()) {
        const PaneNode* node = find_node(root_.get(), zoomed_pane_id_.value());
        if (node) {
            PaneLayoutItem item;
            item.pane_id = node->pane_id;
            item.title = node->title;
            item.cwd = node->cwd;
            item.bounds = PaneRect{0, 0, total_width, total_height};
            item.is_focused = true;
            item.is_pinned = node->is_pinned;
            items.push_back(item);
            return items;
        }
    }

    layout_recursive(root_.get(), PaneRect{0, 0, total_width, total_height}, items);
    return items;
}

std::optional<uint32_t> PaneTree::find_adjacent_pane(uint32_t from_id, NavigationDirection dir, int total_w, int total_h) const {
    auto layout = compute_layout(total_w, total_h);
    const PaneLayoutItem* current = nullptr;
    for (const auto& item : layout) {
        if (item.pane_id == from_id) {
            current = &item;
            break;
        }
    }
    if (!current) return std::nullopt;

    int cur_cx = current->bounds.x + current->bounds.width / 2;
    int cur_cy = current->bounds.y + current->bounds.height / 2;

    uint32_t best_id = 0;
    int min_dist = 9999999;

    for (const auto& item : layout) {
        if (item.pane_id == from_id) continue;
        int cx = item.bounds.x + item.bounds.width / 2;
        int cy = item.bounds.y + item.bounds.height / 2;

        bool valid = false;
        switch (dir) {
            case NavigationDirection::Left:  valid = cx < cur_cx; break;
            case NavigationDirection::Right: valid = cx > cur_cx; break;
            case NavigationDirection::Up:    valid = cy < cur_cy; break;
            case NavigationDirection::Down:  valid = cy > cur_cy; break;
        }

        if (valid) {
            int dist = (cx - cur_cx) * (cx - cur_cx) + (cy - cur_cy) * (cy - cur_cy);
            if (dist < min_dist) {
                min_dist = dist;
                best_id = item.pane_id;
            }
        }
    }

    if (best_id != 0) return best_id;
    return std::nullopt;
}

PaneTree& get_session_pane_tree() {
    static PaneTree s_session_tree;
    return s_session_tree;
}

} // namespace meridian::workspace

