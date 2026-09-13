#pragma once
// meridian-terminal / workspace / session_recorder.hpp
//
// Reproducible session recording and replay engine. Records time-indexed
// PTY inputs, outputs, terminal resize events, and process exit codes.

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace meridian::workspace {

enum class SessionEventType : uint8_t {
    KeyInput,
    PtyOutput,
    Resize,
    CommandStart,
    CommandFinish
};

struct SessionEvent {
    uint64_t timestamp_ms = 0;
    SessionEventType type = SessionEventType::PtyOutput;
    uint32_t pane_id = 1;
    std::string payload;
};

class SessionRecorder {
public:
    SessionRecorder();

    void start();
    void stop();
    bool is_recording() const { return recording_; }

    void record_event(SessionEventType type, uint32_t pane_id, const std::string& payload);

    bool save_to_file(const std::string& file_path) const;
    bool load_from_file(const std::string& file_path);

    std::size_t event_count() const { return events_.size(); }
    uint64_t duration_ms() const;
    const std::vector<SessionEvent>& events() const { return events_; }
    void clear();

private:
    bool recording_ = false;
    uint64_t start_time_ms_ = 0;
    std::vector<SessionEvent> events_;

    static uint64_t current_time_ms();
};

} // namespace meridian::workspace

