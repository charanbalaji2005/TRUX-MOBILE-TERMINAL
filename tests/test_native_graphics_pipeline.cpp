// tests/test_native_graphics_pipeline.cpp
#include "mini_test.hpp"
#include "../src/core/graphics/image_decoder.hpp"
#include "../src/core/graphics/gpu_texture_manager.hpp"
#include "../src/core/graphics/terminal_image_compositor.hpp"
#include "../src/core/graphics/graphics_protocol_parser.hpp"
#include "../src/core/graphics/graphics_manager.hpp"
#include "../src/shell/shell.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

using namespace meridian::graphics;

MTEST(native_graphics_decoder_rgba8_format) {
    // Decode real gallery PNG image
    auto decoded = ImageDecoder::decode_file("resources/images/gallery/sharingan_eye.png");
    ASSERT_TRUE(decoded.is_valid());
    ASSERT_TRUE(decoded.original_width > 0);
    ASSERT_TRUE(decoded.original_height > 0);
    ASSERT_EQ(decoded.frames.size(), 1UL);

    const auto& frame = decoded.frame(0);
    ASSERT_EQ(frame.rgba.size(), static_cast<size_t>(frame.width * frame.height * 4));
}

MTEST(native_graphics_gpu_texture_creation_and_caching) {
    GpuTextureManager tm;
    auto decoded = ImageDecoder::decode_file("resources/images/gallery/sharingan_eye.png");
    ASSERT_TRUE(decoded.is_valid());

    auto tex1 = tm.get_or_create_texture("test_tex", decoded);
    ASSERT_TRUE(tex1 != nullptr);
    ASSERT_TRUE(tex1->id > 0);
    ASSERT_TRUE(tex1->is_uploaded);
    ASSERT_EQ(tex1->width, decoded.original_width);
    ASSERT_EQ(tex1->height, decoded.original_height);

    // Second request with same key should return cached instance
    auto tex2 = tm.get_or_create_texture("test_tex", decoded);
    ASSERT_EQ(tex1, tex2);
    ASSERT_EQ(tm.total_cached_textures(), 1UL);
}

MTEST(native_graphics_damage_tracking) {
    GpuTextureManager tm;
    ASSERT_FALSE(tm.has_damage());

    std::vector<uint8_t> dummy(64 * 64 * 4, 255);
    auto tex = tm.create_texture_from_rgba("dummy", dummy.data(), 64, 64);
    ASSERT_TRUE(tex != nullptr);
    ASSERT_TRUE(tm.has_damage());

    auto damage = tm.consume_damage();
    ASSERT_FALSE(damage.empty());
    ASSERT_FALSE(tm.has_damage());
}

MTEST(native_graphics_compositor_aspect_fit_contain) {
    float out_w = 0, out_h = 0, u0 = 0, v0 = 0, u1 = 0, v1 = 0;
    // Source: 1000x500 (2:1), Target Box: 500x500 (1:1)
    TerminalImageCompositor::compute_aspect_fit(
        1000, 500,
        500.0f, 500.0f,
        ImageFitMode::Contain,
        out_w, out_h,
        u0, v0, u1, v1
    );

    ASSERT_EQ(static_cast<int>(out_w), 500);
    ASSERT_EQ(static_cast<int>(out_h), 250);
    ASSERT_EQ(u0, 0.0f);
    ASSERT_EQ(u1, 1.0f);
}

MTEST(native_graphics_compositor_aspect_fit_cover) {
    float out_w = 0, out_h = 0, u0 = 0, v0 = 0, u1 = 0, v1 = 0;
    // Source: 1000x500 (2:1), Target Box: 500x500 (1:1) -> Cover fills 500x500 and crops width
    TerminalImageCompositor::compute_aspect_fit(
        1000, 500,
        500.0f, 500.0f,
        ImageFitMode::Cover,
        out_w, out_h,
        u0, v0, u1, v1
    );

    ASSERT_EQ(static_cast<int>(out_w), 500);
    ASSERT_EQ(static_cast<int>(out_h), 500);
    ASSERT_TRUE(u0 > 0.0f); // Cropped
    ASSERT_TRUE(u1 < 1.0f);
}

MTEST(native_graphics_compositor_percentage_scaling) {
    TerminalImageCompositor comp;
    TerminalCellMetrics m;
    m.columns = 100;
    m.rows = 50;
    m.cell_width_px = 10.0f;  // Canvas width: 1000px
    m.cell_height_px = 20.0f; // Canvas height: 1000px
    comp.set_metrics(m);

    ImageObject img;
    img.original_width = 800;
    img.original_height = 400; // 2:1 aspect
    img.width_percentage = 50.0f; // 50% of 1000px = 500px
    img.fit_mode = ImageFitMode::Contain;

    auto bounds = comp.compute_render_bounds(img);
    ASSERT_TRUE(bounds.visible);
    ASSERT_EQ(static_cast<int>(bounds.width), 500);
    ASSERT_EQ(static_cast<int>(bounds.height), 250);
}

MTEST(native_graphics_compositor_scrollback_tracking) {
    TerminalImageCompositor comp;
    TerminalCellMetrics m;
    m.cell_height_px = 20.0f;
    comp.set_metrics(m);

    std::vector<ImageObject> images;
    ImageObject img;
    img.id = 1;
    img.y = 200.0f;
    img.scroll_with_terminal = true;
    img.placement = ImagePlacementType::CursorRelative;
    images.push_back(img);

    // Scroll by 3 lines (3 * 20px = 60px upwards)
    comp.on_terminal_scroll(3, images);
    ASSERT_EQ(static_cast<int>(images[0].y), 140);
}

MTEST(native_graphics_compositor_resize_handling) {
    TerminalImageCompositor comp;
    TerminalCellMetrics m;
    m.columns = 80;
    m.rows = 24;
    m.cell_width_px = 9.0f;
    m.cell_height_px = 18.0f;
    comp.set_metrics(m);

    comp.handle_resize(120, 40, 10.0f, 20.0f);
    ASSERT_EQ(comp.metrics().columns, 120);
    ASSERT_EQ(comp.metrics().rows, 40);
    ASSERT_EQ(static_cast<int>(comp.metrics().cell_width_px), 10);
}

MTEST(native_graphics_protocol_parser_kitty) {
    GraphicsProtocolParser parser;
    std::string kitty_seq = "\033_Ga=T,f=100,s=800,v=600,c=40,r=20,m=0;iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=\033\\";

    ASSERT_EQ(GraphicsProtocolParser::detect_protocol(kitty_seq), GraphicsProtocolType::Kitty);

    ParsedGraphicsCommand cmd;
    bool ok = parser.parse_kitty_sequence(kitty_seq, cmd);
    ASSERT_TRUE(ok);
    ASSERT_EQ(cmd.protocol, GraphicsProtocolType::Kitty);
    ASSERT_EQ(cmd.action, "transmit");
    ASSERT_EQ(cmd.format, 100);
    ASSERT_EQ(cmd.width, 800);
    ASSERT_EQ(cmd.height, 600);
    ASSERT_EQ(cmd.cols, 40);
    ASSERT_EQ(cmd.rows, 20);
    ASSERT_FALSE(cmd.more_chunks);
    ASSERT_FALSE(cmd.payload.empty());
}

MTEST(native_graphics_protocol_parser_iterm2) {
    GraphicsProtocolParser parser;
    std::string iterm_seq = "\033]1337;File=inline=1;width=500;preserveAspectRatio=1:iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=\007";

    ASSERT_EQ(GraphicsProtocolParser::detect_protocol(iterm_seq), GraphicsProtocolType::ITerm2);

    ParsedGraphicsCommand cmd;
    bool ok = parser.parse_iterm2_sequence(iterm_seq, cmd);
    ASSERT_TRUE(ok);
    ASSERT_EQ(cmd.protocol, GraphicsProtocolType::ITerm2);
    ASSERT_TRUE(cmd.is_inline);
    ASSERT_EQ(cmd.width, 500);
    ASSERT_TRUE(cmd.preserve_aspect);
    ASSERT_FALSE(cmd.payload.empty());
}

MTEST(native_graphics_manager_add_and_clear) {
    auto& gm = GraphicsManager::instance();
    gm.clear_all_images();
    ASSERT_EQ(gm.image_count(), 0UL);

    ImageObject cfg;
    cfg.display_width = 300.0f;
    cfg.fit_mode = ImageFitMode::Contain;

    uint64_t id = gm.add_image_file("resources/images/gallery/sharingan_eye.png", cfg);
    ASSERT_TRUE(id > 0);
    ASSERT_EQ(gm.image_count(), 1UL);

    const auto* img = gm.get_image(id);
    ASSERT_TRUE(img != nullptr);
    ASSERT_EQ(img->id, id);

    bool removed = gm.remove_image(id);
    ASSERT_TRUE(removed);
    ASSERT_EQ(gm.image_count(), 0UL);
}

MTEST(native_graphics_manager_debug_report) {
    auto& gm = GraphicsManager::instance();
    ImageDebugReport report;
    bool ok = gm.generate_debug_report("resources/images/gallery/sharingan_eye.png", report);
    ASSERT_TRUE(ok);
    ASSERT_EQ(report.decoded_format, "RGBA8");
    ASSERT_TRUE(report.original_width > 0);
    ASSERT_TRUE(report.original_height > 0);
    ASSERT_TRUE(report.gpu_enabled);
}

MTEST(native_graphics_corrupted_image_handling) {
    // Attempting to decode invalid/corrupted data must not crash
    std::vector<uint8_t> junk = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x11, 0x22, 0x33};
    auto decoded = ImageDecoder::decode_memory(junk.data(), junk.size(), "corrupted.png");
    ASSERT_FALSE(decoded.is_valid());
}

MTEST(native_graphics_cli_pic_clear_and_list) {
    meridian::shell::Shell shell(false);
    std::ostringstream err1;
    int rc1 = shell.run_command("pic --list", err1);
    ASSERT_EQ(rc1, 0);

    std::ostringstream err2;
    int rc2 = shell.run_command("pic --clear", err2);
    ASSERT_EQ(rc2, 0);
}
