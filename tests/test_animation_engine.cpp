// tests/test_animation_engine.cpp
#include "mini_test.hpp"
#include "../src/core/graphics/animation_frame.hpp"
#include "../src/core/graphics/animation_controller.hpp"
#include "../src/core/graphics/animation_engine.hpp"
#include "../src/core/graphics/graphics_protocol_parser.hpp"

using namespace meridian::graphics;

MTEST(animation_frame_creation_and_rescale) {
    std::vector<uint8_t> pixels(4 * 4 * 4, 255); // 4x4 white RGBA
    AnimationFrame frame(4, 4, pixels, 0.05);

    ASSERT_TRUE(frame.is_valid());
    ASSERT_EQ(frame.width(), 4);
    ASSERT_EQ(frame.height(), 4);
    ASSERT_GT(frame.duration(), 0.04);
    ASSERT_EQ(frame.byte_size(), 64u);

    // Rescale to 8x8
    AnimationFrame scaled = frame.rescale(8, 8);
    ASSERT_TRUE(scaled.is_valid());
    ASSERT_EQ(scaled.width(), 8);
    ASSERT_EQ(scaled.height(), 8);
    ASSERT_EQ(scaled.byte_size(), 256u);
}

MTEST(animation_controller_playback_lifecycle) {
    std::vector<AnimationFrame> frames;
    for (int i = 0; i < 4; ++i) {
        std::vector<uint8_t> px(2 * 2 * 4, static_cast<uint8_t>(i * 50));
        frames.emplace_back(2, 2, px, 0.1);
    }

    AnimationController ctrl;
    ctrl.set_frames(frames);

    ASSERT_EQ(ctrl.frame_count(), 4u);
    ASSERT_GT(ctrl.total_duration(), 0.39);
    ASSERT_TRUE(ctrl.is_stopped());

    ctrl.play();
    ASSERT_TRUE(ctrl.is_playing());
    ASSERT_EQ(ctrl.current_frame_index(), 0u);

    // Advance 0.15s -> frame should advance to 1
    bool changed = ctrl.tick(0.15);
    ASSERT_TRUE(changed);
    ASSERT_EQ(ctrl.current_frame_index(), 1u);

    // Pause
    ctrl.pause();
    ASSERT_TRUE(ctrl.is_paused());
    bool changed_while_paused = ctrl.tick(0.2);
    ASSERT_FALSE(changed_while_paused);

    // Seek
    ctrl.seek_frame(3);
    ASSERT_EQ(ctrl.current_frame_index(), 3u);

    // Stop
    ctrl.stop();
    ASSERT_TRUE(ctrl.is_stopped());
    ASSERT_EQ(ctrl.current_frame_index(), 0u);
}

MTEST(animation_controller_loop_limits) {
    std::vector<AnimationFrame> frames;
    for (int i = 0; i < 2; ++i) {
        std::vector<uint8_t> px(2 * 2 * 4, 100);
        frames.emplace_back(2, 2, px, 0.1);
    }

    AnimationController ctrl;
    ctrl.set_frames(frames);
    ctrl.set_loop_count(2); // Loop twice then stop
    ctrl.play();

    // Pass 1 loop (0.2s)
    ctrl.tick(0.25);
    ASSERT_TRUE(ctrl.is_playing());

    // Pass 2nd loop (0.2s more)
    ctrl.tick(0.25);
    ASSERT_TRUE(ctrl.is_finished());
    ASSERT_TRUE(ctrl.is_stopped());
}

MTEST(animation_engine_add_and_tick) {
    auto& eng = AnimationEngine::instance();

    uint64_t anim_id = eng.create_empty_animation(10, 10, "test_anim");
    ASSERT_GT(anim_id, 0u);

    std::vector<uint8_t> p1(10 * 10 * 4, 100);
    std::vector<uint8_t> p2(10 * 10 * 4, 200);

    AnimationFrame f1(10, 10, p1, 0.1);
    AnimationFrame f2(10, 10, p2, 0.1);

    ASSERT_TRUE(eng.add_frame(anim_id, f1));
    ASSERT_TRUE(eng.add_frame(anim_id, f2));
    ASSERT_EQ(eng.frame_count(anim_id), 2u);

    eng.play(anim_id);
    auto changed = eng.tick(0.15);
    ASSERT_FALSE(changed.empty());

    const auto& canvas = eng.get_composed_canvas(anim_id);
    ASSERT_EQ(canvas.size(), 400u);

    eng.remove_animation(anim_id);
}

MTEST(animation_engine_limits_and_security) {
    auto& eng = AnimationEngine::instance();

    // Verify default limits
    ASSERT_GE(eng.limits().max_memory_bytes, 1024 * 1024u);
    ASSERT_GE(eng.limits().max_width, 1024);
    ASSERT_GE(eng.limits().max_height, 1024);

    // Over-limit dimensions should fail
    uint64_t bad_id = eng.create_empty_animation(50000, 50000, "huge_bomb");
    ASSERT_EQ(bad_id, 0u);
}

MTEST(graphics_protocol_parser_kitty_animation) {
    GraphicsProtocolParser parser;
    ParsedGraphicsCommand cmd;

    // Kitty animation sequence: a=a,s=100,v=100,c=2,r=5,z=50; payload
    std::string seq = "\033_Ga=a,s=100,v=100,c=2,r=5,z=50;\033\\";
    bool ok = parser.parse_kitty_sequence(seq, cmd);

    ASSERT_TRUE(ok);
    ASSERT_TRUE(cmd.is_animation);
    ASSERT_EQ(cmd.action, "animate");
    ASSERT_EQ(cmd.width, 100);
    ASSERT_EQ(cmd.height, 100);
    ASSERT_EQ(cmd.frame_index, 2);
    ASSERT_EQ(cmd.loop_count, 5);
    ASSERT_EQ(cmd.frame_delay_ms, 50);
}

