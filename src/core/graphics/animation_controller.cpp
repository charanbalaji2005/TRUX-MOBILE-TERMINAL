// src/core/graphics/animation_controller.cpp
#include "animation_controller.hpp"

#include <algorithm>
#include <cmath>

namespace meridian::graphics {

AnimationController::AnimationController() = default;

void AnimationController::set_frames(const std::vector<AnimationFrame>& frames) {
    frame_durations_.clear();
    for (const auto& f : frames) {
        double d = (f.duration() > 0.001) ? f.duration() : 0.1;
        frame_durations_.push_back(d);
    }
    recalculate_total_duration();
    current_frame_index_ = 0;
    current_time_ = 0.0;
    loops_completed_ = 0;
    finished_ = false;
}

void AnimationController::set_loop_count(int loops) {
    loop_count_ = (loops >= 0) ? loops : 0;
}

void AnimationController::set_fps_override(double fps) {
    fps_override_ = fps;
    recalculate_total_duration();
}

void AnimationController::recalculate_total_duration() {
    total_duration_ = 0.0;
    if (frame_durations_.empty()) return;

    if (fps_override_ > 0.1) {
        total_duration_ = frame_durations_.size() / fps_override_;
    } else {
        for (double d : frame_durations_) {
            total_duration_ += d;
        }
    }
}

void AnimationController::play() {
    if (finished_) {
        seek_frame(0);
        loops_completed_ = 0;
        finished_ = false;
    }
    state_ = PlaybackState::Playing;
}

void AnimationController::pause() {
    state_ = PlaybackState::Paused;
}

void AnimationController::stop() {
    state_ = PlaybackState::Stopped;
    current_time_ = 0.0;
    current_frame_index_ = 0;
    loops_completed_ = 0;
    finished_ = false;
}

void AnimationController::toggle() {
    if (state_ == PlaybackState::Playing) pause();
    else play();
}

void AnimationController::seek_frame(size_t index) {
    if (frame_durations_.empty()) return;
    current_frame_index_ = std::min(index, frame_durations_.size() - 1);

    // Calculate elapsed time up to this frame
    current_time_ = 0.0;
    if (fps_override_ > 0.1) {
        current_time_ = current_frame_index_ / fps_override_;
    } else {
        for (size_t i = 0; i < current_frame_index_; ++i) {
            current_time_ += frame_durations_[i];
        }
    }
}

void AnimationController::seek_time(double seconds) {
    if (total_duration_ <= 0.0) return;
    current_time_ = std::clamp(seconds, 0.0, total_duration_);
    update_frame_from_time();
}

void AnimationController::step_forward() {
    if (frame_durations_.empty()) return;
    size_t next_idx = (current_frame_index_ + 1) % frame_durations_.size();
    seek_frame(next_idx);
}

void AnimationController::step_backward() {
    if (frame_durations_.empty()) return;
    size_t prev_idx = (current_frame_index_ == 0) ? (frame_durations_.size() - 1) : (current_frame_index_ - 1);
    seek_frame(prev_idx);
}

void AnimationController::update_frame_from_time() {
    if (frame_durations_.empty()) return;

    if (fps_override_ > 0.1) {
        size_t idx = static_cast<size_t>(std::floor(current_time_ * fps_override_));
        current_frame_index_ = std::min(idx, frame_durations_.size() - 1);
        return;
    }

    double accum = 0.0;
    for (size_t i = 0; i < frame_durations_.size(); ++i) {
        accum += frame_durations_[i];
        if (current_time_ < accum || i == frame_durations_.size() - 1) {
            current_frame_index_ = i;
            break;
        }
    }
}

bool AnimationController::tick(double dt) {
    if (state_ != PlaybackState::Playing || frame_durations_.empty() || total_duration_ <= 0.0) {
        return false;
    }

    size_t old_index = current_frame_index_;
    current_time_ += dt;

    if (current_time_ >= total_duration_) {
        loops_completed_++;
        if (loop_count_ > 0 && loops_completed_ >= loop_count_) {
            // Reached finite loop limit
            current_time_ = total_duration_;
            current_frame_index_ = frame_durations_.size() - 1;
            state_ = PlaybackState::Stopped;
            finished_ = true;
            return current_frame_index_ != old_index;
        }

        // Loop animation
        current_time_ = std::fmod(current_time_, total_duration_);
    }

    update_frame_from_time();
    return current_frame_index_ != old_index;
}

double AnimationController::current_frame_duration() const {
    if (frame_durations_.empty()) return 0.1;
    if (fps_override_ > 0.1) return 1.0 / fps_override_;
    return frame_durations_[current_frame_index_ % frame_durations_.size()];
}

} // namespace meridian::graphics

