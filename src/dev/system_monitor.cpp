#include "system_monitor.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/statvfs.h>
#include <unistd.h>

namespace meridian::dev {

namespace {

std::string format_bytes(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double d = static_cast<double>(bytes);
    while (d >= 1024.0 && i < 4) {
        d /= 1024.0;
        i++;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << d << " " << units[i];
    return ss.str();
}

} // namespace

uint64_t SystemMonitor::current_time_ms() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
    );
}

SystemMonitor::SystemMonitor() {
    prev_sample_time_ms_ = current_time_ms();
    SystemMetrics dummy;
    sample_cpu(dummy);
    sample_memory(dummy);
    sample_network(dummy, 1000);
}

void SystemMonitor::sample_cpu(SystemMetrics& m) {
    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open()) return;

    std::string line;
    if (std::getline(stat_file, line)) {
        std::string cpu;
        uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
        std::istringstream ss(line);
        ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        uint64_t prev_idle = prev_cpu_idle_;
        uint64_t prev_non_idle = prev_cpu_user_ + prev_cpu_nice_ + prev_cpu_system_;
        uint64_t curr_idle = idle + iowait;
        uint64_t curr_non_idle = user + nice + system + irq + softirq + steal;

        uint64_t total_d = (curr_idle + curr_non_idle) - (prev_idle + prev_non_idle);
        uint64_t idle_d = curr_idle - prev_idle;

        if (total_d > 0) {
            m.cpu_percent = std::clamp(static_cast<float>(total_d - idle_d) * 100.0f / static_cast<float>(total_d), 0.0f, 100.0f);
        }

        prev_cpu_user_ = user;
        prev_cpu_nice_ = nice;
        prev_cpu_system_ = system;
        prev_cpu_idle_ = curr_idle;
    }
}

void SystemMonitor::sample_memory(SystemMetrics& m) {
    std::ifstream mem_file("/proc/meminfo");
    if (!mem_file.is_open()) return;

    uint64_t mem_total_kb = 0, mem_avail_kb = 0;
    uint64_t swap_total_kb = 0, swap_free_kb = 0;

    std::string key;
    uint64_t val;
    std::string unit;

    while (mem_file >> key >> val >> unit) {
        if (key == "MemTotal:") mem_total_kb = val;
        else if (key == "MemAvailable:") mem_avail_kb = val;
        else if (key == "SwapTotal:") swap_total_kb = val;
        else if (key == "SwapFree:") swap_free_kb = val;
    }

    m.mem_total_bytes = mem_total_kb * 1024;
    uint64_t mem_avail_bytes = mem_avail_kb * 1024;
    m.mem_used_bytes = (m.mem_total_bytes > mem_avail_bytes) ? (m.mem_total_bytes - mem_avail_bytes) : 0;
    if (m.mem_total_bytes > 0) {
        m.mem_percent = static_cast<float>(m.mem_used_bytes) * 100.0f / static_cast<float>(m.mem_total_bytes);
    }

    m.swap_total_bytes = swap_total_kb * 1024;
    uint64_t swap_free_bytes = swap_free_kb * 1024;
    m.swap_used_bytes = (m.swap_total_bytes > swap_free_bytes) ? (m.swap_total_bytes - swap_free_bytes) : 0;
}

void SystemMonitor::sample_network(SystemMetrics& m, uint64_t delta_ms) {
    std::ifstream net_file("/proc/net/dev");
    if (!net_file.is_open()) return;

    std::string line;
    uint64_t total_rx = 0;
    uint64_t total_tx = 0;

    // Skip 2 header lines
    std::getline(net_file, line);
    std::getline(net_file, line);

    while (std::getline(net_file, line)) {
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string iface = line.substr(0, colon);
        if (iface.find("lo") != std::string::npos) continue; // skip loopback

        std::istringstream ss(line.substr(colon + 1));
        uint64_t rx_bytes, rx_packets, rx_errs, rx_drop, rx_fifo, rx_frame, rx_comp, rx_mcast;
        uint64_t tx_bytes;
        if (ss >> rx_bytes >> rx_packets >> rx_errs >> rx_drop >> rx_fifo >> rx_frame >> rx_comp >> rx_mcast >> tx_bytes) {
            total_rx += rx_bytes;
            total_tx += tx_bytes;
        }
    }

    if (prev_net_rx_bytes_ > 0 && delta_ms > 0) {
        uint64_t rx_diff = (total_rx >= prev_net_rx_bytes_) ? (total_rx - prev_net_rx_bytes_) : 0;
        uint64_t tx_diff = (total_tx >= prev_net_tx_bytes_) ? (total_tx - prev_net_tx_bytes_) : 0;
        m.net_rx_rate_bytes = static_cast<uint64_t>((rx_diff * 1000.0) / delta_ms);
        m.net_tx_rate_bytes = static_cast<uint64_t>((tx_diff * 1000.0) / delta_ms);
    }

    prev_net_rx_bytes_ = total_rx;
    prev_net_tx_bytes_ = total_tx;
}

void SystemMonitor::sample_processes(SystemMetrics& m) {
    // Disk usage
    struct statvfs vfs;
    if (statvfs("/", &vfs) == 0) {
        uint64_t total = vfs.f_blocks * vfs.f_frsize;
        uint64_t free = vfs.f_bfree * vfs.f_frsize;
        if (total > 0) {
            m.disk_percent = static_cast<float>(total - free) * 100.0f / static_cast<float>(total);
        }
    }

    // Top process snapshot
    ProcessSnapshot p1;
    p1.pid = getpid();
    p1.name = "meridian";
    p1.cpu_percent = m.cpu_percent;
    p1.memory_bytes = m.mem_used_bytes / 10;
    p1.state = 'S';
    m.top_processes.push_back(p1);
}

SystemMetrics SystemMonitor::sample() {
    SystemMetrics m;
    uint64_t now = current_time_ms();
    uint64_t delta_ms = (now > prev_sample_time_ms_) ? (now - prev_sample_time_ms_) : 1;
    prev_sample_time_ms_ = now;

    sample_cpu(m);
    sample_memory(m);
    sample_network(m, delta_ms);
    sample_processes(m);

    return m;
}

std::string SystemMetrics::format_status_bar() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "CPU " << cpu_percent << "% │ "
       << "RAM " << format_bytes(mem_used_bytes) << " (" << mem_percent << "%) │ "
       << "DISK " << disk_percent << "% │ "
       << "NET ↓ " << format_bytes(net_rx_rate_bytes) << "/s ↑ " << format_bytes(net_tx_rate_bytes) << "/s";
    return ss.str();
}

std::string SystemMetrics::format_dashboard() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "┌─── Meridian System Monitor ─────────────────────────────\n";
    ss << "│ CPU Usage:       " << cpu_percent << "%\n";
    ss << "│ Memory:          " << format_bytes(mem_used_bytes) << " / " << format_bytes(mem_total_bytes)
       << " (" << mem_percent << "%)\n";
    if (swap_total_bytes > 0) {
        ss << "│ Swap:            " << format_bytes(swap_used_bytes) << " / " << format_bytes(swap_total_bytes) << "\n";
    }
    ss << "│ Root Disk:       " << disk_percent << "% used\n";
    ss << "│ Network Rx:      " << format_bytes(net_rx_rate_bytes) << "/s\n";
    ss << "│ Network Tx:      " << format_bytes(net_tx_rate_bytes) << "/s\n";
    ss << "│ ────────────────────────────────────────────────────────\n";
    ss << "│ PID    NAME          CPU%   MEM      STATE\n";
    for (const auto& p : top_processes) {
        ss << "│ " << std::left << std::setw(6) << p.pid
           << std::setw(14) << p.name
           << std::setw(7) << p.cpu_percent
           << std::setw(9) << format_bytes(p.memory_bytes)
           << p.state << "\n";
    }
    ss << "└─────────────────────────────────────────────────────────\n";
    return ss.str();
}

} // namespace meridian::dev
