cmake_minimum_required(VERSION 3.20)
project(meridian-terminal
    VERSION 2.0.0
    DESCRIPTION "Meridian Terminal — Modern Linux Terminal Emulator & Unified Developer Canvas"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

add_compile_options(-Wall -Wextra -O2)

option(MERIDIAN_BUILD_TESTS "Build the Meridian test suite" ON)
option(MERIDIAN_ENABLE_ASAN "Build with AddressSanitizer + UBSan" OFF)

if(MERIDIAN_ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

# ---- meridian-core: Linux PTY + VT/ANSI engine + GPU renderer ------------
add_library(meridian-core STATIC
    src/core/pty/pty_manager.cpp
    src/core/pty/pty_session.cpp
    src/core/vt/screen_buffer.cpp
    src/core/vt/ansi_parser.cpp
    src/core/vt/graphics.cpp
    src/core/graphics/image_decoder.cpp
    src/core/graphics/terminal_graphic.cpp
    src/core/config.cpp
    src/core/terminal_image.cpp
    src/core/art_gallery.cpp
    src/core/renderer/damage_tracker.cpp
    src/core/renderer/glyph_atlas.cpp
    src/core/renderer/render_pipeline.cpp
    src/platform/LinuxPTY.cpp
)
target_include_directories(meridian-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-core PUBLIC util)

# ---- meridian-shell-lib: lexer/parser/executor/builtins/Shell/LineEditor -
add_library(meridian-shell-lib STATIC
    src/shell/lexer.cpp
    src/shell/parser.cpp
    src/shell/executor.cpp
    src/shell/builtins.cpp
    src/shell/line_editor.cpp
    src/shell/shell.cpp
)
target_include_directories(meridian-shell-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-shell-lib PUBLIC util meridian-core meridian-ai-lib meridian-dev-lib)

# ---- meridian-workspace-lib: multiplexer panes + persistent workspaces ---
add_library(meridian-workspace-lib STATIC
    src/workspace/pane_tree.cpp
    src/workspace/workspace_manager.cpp
    src/workspace/session_recorder.cpp
)
target_include_directories(meridian-workspace-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-workspace-lib PUBLIC meridian-core)

# ---- meridian-dev-lib: developer intelligence suite ----------------------
add_library(meridian-dev-lib STATIC
    src/dev/git_intel.cpp
    src/dev/system_monitor.cpp
    src/dev/file_explorer.cpp
    src/dev/command_palette.cpp
    src/dev/universal_search.cpp
    src/dev/rich_history.cpp
)
target_include_directories(meridian-dev-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-dev-lib PUBLIC meridian-core)

# ---- meridian-ai-lib: local-only Meridian AI layer -----------------------
add_library(meridian-ai-lib STATIC
    src/ai/secret_redactor.cpp
    src/ai/risk_classifier.cpp
    src/ai/command_analyzer.cpp
    src/ai/command_context.cpp
    src/ai/ai_controller.cpp
    src/ai/intent_engine.cpp
    src/ai/error_diagnostics.cpp
    src/ai/ai_agent.cpp
)
target_include_directories(meridian-ai-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-ai-lib PUBLIC meridian-core)

# ---- meridian-config-lib: terminal settings + keybindings ----------------
add_library(meridian-config-lib STATIC
    src/config/terminal_config.cpp
    src/config/keybindings.cpp
)
target_include_directories(meridian-config-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(meridian-config-lib PUBLIC meridian-core)

# ---- meridian-security-lib: credential storage ---------------------------
add_library(meridian-security-lib STATIC
    src/security/credential_store.cpp
)
target_include_directories(meridian-security-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- meridian: the unified Meridian 2.0 CLI & Terminal Shell -------------
add_executable(meridian
    src/app/meridian_gui.cpp
    src/app/meridian_main.cpp
)
target_link_libraries(meridian PRIVATE
    meridian-core
    meridian-shell-lib
    meridian-ai-lib
    meridian-workspace-lib
    meridian-dev-lib
    meridian-config-lib
    meridian-security-lib
    util
)

# ---- meridian-shell: standalone shell binary ------------------------------
add_executable(meridian-shell src/app/shell_main.cpp)
target_link_libraries(meridian-shell PRIVATE meridian-shell-lib util)

# ---- meridian_demo: headless PTY + ANSI integration demo -----------------
add_executable(meridian_demo src/app/demo_main.cpp)
target_link_libraries(meridian_demo PRIVATE meridian-core util)

# ---- Tests -----------------------------------------------------------------
if(MERIDIAN_BUILD_TESTS)
    enable_testing()
    add_executable(meridian_tests
        tests/test_main.cpp
        tests/test_screen_buffer.cpp
        tests/test_ansi_parser.cpp
        tests/test_pty_manager.cpp
        tests/test_shell_lexer.cpp
        tests/test_shell_parser.cpp
        tests/test_shell_executor.cpp
        tests/test_job_control.cpp
        tests/test_shell_class.cpp
        tests/test_ai.cpp
        tests/test_command_context.cpp
        tests/test_config.cpp
        tests/test_security.cpp
        tests/test_renderer.cpp
        tests/test_workspace.cpp
        tests/test_intent_and_diagnostics.cpp
        tests/test_dev_tools.cpp
    )
    target_link_libraries(meridian_tests PRIVATE
        meridian-core meridian-shell-lib meridian-ai-lib
        meridian-workspace-lib meridian-dev-lib
        meridian-config-lib meridian-security-lib
        util
    )
    add_test(NAME meridian_tests COMMAND meridian_tests)
    add_test(NAME meridian_manual_core_test
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/manual_core_test.sh
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# ---- CPack Multi-Platform Linux Packaging -----------------------------------
install(TARGETS meridian meridian-shell RUNTIME DESTINATION bin)
install(FILES meridian.desktop DESTINATION share/applications)
install(FILES LICENSE README.md DESTINATION share/doc/meridian-terminal)

set(CPACK_PACKAGE_NAME "meridian-terminal")
set(CPACK_PACKAGE_VENDOR "Meridian Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern Linux Terminal & Unified Developer Environment")
set(CPACK_PACKAGE_VERSION "2.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_CONTACT "Charan Balaji <charanbalaji@meridian.dev>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_GENERATOR "TGZ;DEB;RPM")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Charan Balaji <charanbalaji@meridian.dev>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")

include(CPack)
