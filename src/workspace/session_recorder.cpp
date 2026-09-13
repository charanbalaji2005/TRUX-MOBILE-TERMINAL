#include "session_recorder.hpp"

#include <chrono>
#include <fstream>
#include <sstream>

namespace meridian::workspace {

uint64_t SessionRecorder::current_time_ms() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
    );
}

SessionRecorder::SessionRecorder() = default;

void SessionRecorder::start() {
    recording_ = true;
    start_time_ms_ = current_time_ms();
    events_.clear();
}

void SessionRecorder::stop() {
    recording_ = false;
}

void SessionRecorder::clear() {
    events_.clear();
    recording_ = false;
    start_time_ms_ = 0;
}

void SessionRecorder::record_event(SessionEventType type, uint32_t pane_id, const std::string& payload) {
    if (!recording_) return;
    SessionEvent ev;
    ev.timestamp_ms = current_time_ms() - start_time_ms_;
    ev.type = type;
    ev.pane_id = pane_id;
    ev.payload = payload;
    events_.push_back(std::move(ev));
}

uint64_t SessionRecorder::duration_ms() const {
    if (events_.empty()) return 0;
    return events_.back().timestamp_ms;
}

bool SessionRecorder::save_to_file(const std::string& file_path) const {
    std::ofstream out(file_path, std::ios::trunc | std::ios::binary);
    if (!out.is_open()) return false;

    out << "MERIDIAN_SESSION_V1\n";
    out << events_.size() << "\n";
    for (const auto& ev : events_) {
        out << ev.timestamp_ms << " "
            << static_cast<int>(ev.type) << " "
            << ev.pane_id << " "
            << ev.payload.size() << "\n";
        out.write(ev.payload.data(), ev.payload.size());
        out << "\n";
    }
    return out.good();
}

bool SessionRecorder::load_from_file(const std::string& file_path) {
    std::ifstream in(file_path, std::ios::binary);
    if (!in.is_open()) return false;

    std::string header;
    if (!std::getline(in, header) || header != "MERIDIAN_SESSION_V1") {
        return false;
    }

    std::size_t count = 0;
    if (!(in >> count)) return false;

    events_.clear();
    events_.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        SessionEvent ev;
        int type_int = 0;
        std::size_t payload_len = 0;

        if (!(in >> ev.timestamp_ms >> type_int >> ev.pane_id >> payload_len)) {
            break;
        }

        // Consume newline
        char c;
        in.get(c);

        ev.type = static_cast<SessionEventType>(type_int);
        ev.payload.resize(payload_len);
        in.read(ev.payload.data(), payload_len);
        in.get(c); // consume trailing newline

        events_.push_back(std::move(ev));
    }

    return true;
}

} // namespace meridian::workspace

