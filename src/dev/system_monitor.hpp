#pragma once
// meridian-terminal / dev / system_monitor.hpp
//
// Lightweight native Linux system & network monitor. Inspects /proc
// to provide real-time CPU usage, RAM utilization, network throughput,
// and process metrics without external heavy daemons.

#include <cstdint>
#include <string>
#include <vector>

namespace meridian::dev {

struct ProcessSnapshot {
    int pid = 0;
    std::string name;
    float cpu_percent = 0.0f;
    uint64_t memory_bytes = 0;
    char state = 'R';
};

struct SystemMetrics {
    float cpu_percent = 0.0f;
    uint64_t mem_total_bytes = 0;
    uint64_t mem_used_bytes = 0;
    float mem_percent = 0.0f;
    uint64_t swap_total_bytes = 0;
    uint64_t swap_used_bytes = 0;
    float disk_percent = 0.0f;
    uint64_t net_rx_rate_bytes = 0;
    uint64_t net_tx_rate_bytes = 0;
    std::vector<ProcessSnapshot> top_processes;

    std::string format_status_bar() const;
    std::string format_dashboard() const;
};

class SystemMonitor {
public:
    SystemMonitor();

    SystemMetrics sample();

private:
    uint64_t prev_cpu_user_ = 0;
    uint64_t prev_cpu_nice_ = 0;
    uint64_t prev_cpu_system_ = 0;
    uint64_t prev_cpu_idle_ = 0;
    uint64_t prev_net_rx_bytes_ = 0;
    uint64_t prev_net_tx_bytes_ = 0;
    uint64_t prev_sample_time_ms_ = 0;

    void sample_cpu(SystemMetrics& m);
    void sample_memory(SystemMetrics& m);
    void sample_network(SystemMetrics& m, uint64_t delta_ms);
    void sample_processes(SystemMetrics& m);
    static uint64_t current_time_ms();
};

} // namespace meridian::dev

