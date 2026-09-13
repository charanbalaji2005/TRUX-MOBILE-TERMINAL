#pragma once
// src/core/graphics/animation_controller.hpp
//
// Playback state machine, frame timing resolver, seeking, and loop counter.

#include "animation_frame.hpp"
#include <vector>
#include <cstddef>

namespace meridian::graphics {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

class AnimationController {
public:
    AnimationController();

    // Configuration
    void set_frames(const std::vector<AnimationFrame>& frames);
    void set_loop_count(int loops);     // 0 = infinite, N = loop N times
    void set_fps_override(double fps);  // <= 0 means use intrinsic frame durations

    // Playback control
    void play();
    void pause();
    void stop();
    void toggle();
    void seek_frame(size_t index);
    void seek_time(double seconds);
    void step_forward();
    void step_backward();

    // Advance time and return true if active frame index changed
    bool tick(double dt);

    // State inspection
    PlaybackState state() const { return state_; }
    bool is_playing() const { return state_ == PlaybackState::Playing; }
    bool is_paused() const { return state_ == PlaybackState::Paused; }
    bool is_stopped() const { return state_ == PlaybackState::Stopped; }
    bool is_finished() const { return finished_; }

    size_t current_frame_index() const { return current_frame_index_; }
    double current_time() const { return current_time_; }
    double total_duration() const { return total_duration_; }
    size_t frame_count() const { return frame_durations_.size(); }
    int loops_completed() const { return loops_completed_; }
    int loop_count() const { return loop_count_; }

    // Retrieve current active frame duration
    double current_frame_duration() const;

private:
    void recalculate_total_duration();
    void update_frame_from_time();

    PlaybackState state_ = PlaybackState::Stopped;
    std::vector<double> frame_durations_;
    double total_duration_ = 0.0;
    double current_time_ = 0.0;
    size_t current_frame_index_ = 0;

    int loop_count_ = 0;        // 0 = infinite
    int loops_completed_ = 0;
    double fps_override_ = 0.0; // <= 0 = use frame duration
    bool finished_ = false;
};

} // namespace meridian::graphics

