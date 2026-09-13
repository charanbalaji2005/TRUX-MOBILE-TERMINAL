#include "mini_test.hpp"
#include "../src/workspace/pane_tree.hpp"
#include "../src/workspace/session_recorder.hpp"
#include "../src/workspace/workspace_manager.hpp"

#include <unistd.h>

using namespace meridian::workspace;

MTEST(pane_tree_split_and_zoom) {
    PaneTree tree;
    ASSERT_EQ(tree.count_panes(), 1u);
    uint32_t root_id = tree.active_pane_id();

    // Split vertically
    uint32_t p2 = tree.split_pane(root_id, SplitDirection::Vertical, 0.5f, "/tmp");
    ASSERT_NE(p2, 0u);
    ASSERT_EQ(tree.count_panes(), 2u);
    ASSERT_EQ(tree.active_pane_id(), p2);

    // Compute layout
    auto layout = tree.compute_layout(100, 50);
    ASSERT_EQ(layout.size(), 2u);
    ASSERT_EQ(layout[0].bounds.width, 50);
    ASSERT_EQ(layout[1].bounds.width, 50);

    // Zooming
    tree.toggle_zoom();
    ASSERT_TRUE(tree.is_zoomed());
    auto zoomed_layout = tree.compute_layout(100, 50);
    ASSERT_EQ(zoomed_layout.size(), 1u);
    ASSERT_EQ(zoomed_layout[0].bounds.width, 100);
    ASSERT_EQ(zoomed_layout[0].bounds.height, 50);

    tree.toggle_zoom();
    ASSERT_FALSE(tree.is_zoomed());

    // Navigation
    auto adj = tree.find_adjacent_pane(p2, NavigationDirection::Left, 100, 50);
    ASSERT_TRUE(adj.has_value());
    ASSERT_EQ(adj.value(), root_id);

    // Close pane
    ASSERT_TRUE(tree.close_pane(p2));
    ASSERT_EQ(tree.count_panes(), 1u);
}

MTEST(workspace_manager_persistence) {
    std::string test_dir = "/tmp/test_meridian_workspaces";
    WorkspaceManager wm(test_dir);

    Workspace ws;
    ws.name = "backend_dev";
    ws.root_dir = "/home/dev/project";
    ws.git_branch = "feature/auth";
    ws.environment_vars["PORT"] = "8080";
    ws.environment_vars["NODE_ENV"] = "development";

    WorkspacePaneConfig p1;
    p1.id = 1;
    p1.title = "Server";
    p1.cwd = "/home/dev/project/api";
    p1.command = "npm run start";

    WorkspacePaneConfig p2;
    p2.id = 2;
    p2.title = "Database";
    p2.cwd = "/home/dev/project/db";
    p2.command = "docker compose up";
    p2.split_direction = SplitDirection::Horizontal;

    ws.panes.push_back(p1);
    ws.panes.push_back(p2);

    ASSERT_TRUE(wm.save_workspace(ws));

    auto loaded = wm.load_workspace("backend_dev");
    ASSERT_TRUE(loaded.has_value());
    ASSERT_EQ(loaded->name, "backend_dev");
    ASSERT_EQ(loaded->root_dir, "/home/dev/project");
    ASSERT_EQ(loaded->git_branch, "feature/auth");
    ASSERT_EQ(loaded->environment_vars.at("PORT"), "8080");
    ASSERT_EQ(loaded->panes.size(), 2u);

    auto list = wm.list_workspaces();
    ASSERT_EQ(list.size(), 1u);
    ASSERT_EQ(list[0], "backend_dev");

    ASSERT_TRUE(wm.delete_workspace("backend_dev"));
    ASSERT_FALSE(wm.load_workspace("backend_dev").has_value());
}

MTEST(session_recorder_save_load) {
    SessionRecorder recorder;
    recorder.start();
    recorder.record_event(SessionEventType::CommandStart, 1, "ls -la");
    recorder.record_event(SessionEventType::PtyOutput, 1, "total 48\ndrwxr-xr-x");
    recorder.record_event(SessionEventType::CommandFinish, 1, "0");
    recorder.stop();

    ASSERT_EQ(recorder.event_count(), 3u);

    std::string test_file = "/tmp/test_meridian_session.rec";
    ASSERT_TRUE(recorder.save_to_file(test_file));

    SessionRecorder loaded;
    ASSERT_TRUE(loaded.load_from_file(test_file));
    ASSERT_EQ(loaded.event_count(), 3u);
    ASSERT_EQ(loaded.events()[0].payload, "ls -la");
    ASSERT_EQ(loaded.events()[1].payload, "total 48\ndrwxr-xr-x");
    ASSERT_EQ(loaded.events()[2].payload, "0");

    unlink(test_file.c_str());
}

