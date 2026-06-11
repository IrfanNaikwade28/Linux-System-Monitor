#include "cpu.h"
#include <fstream>
#include <sstream>

/*
 * /proc/stat format (first line, the "cpu" aggregate line):
 *
 *   cpu  user nice system idle iowait irq softirq steal [guest guest_nice]
 *
 * Each value is a cumulative count of jiffies (typically 1/100th second)
 * spent in that CPU state since boot.
 *
 * To compute CPU usage between two snapshots:
 *   active_delta = (total_time2 - idle_time2) - (total_time1 - idle_time1)
 *   total_delta  = total_time2 - total_time1
 *   usage        = (active_delta / total_delta) * 100
 *
 * idle_time  = idle + iowait
 * active_time = user + nice + system + irq + softirq + steal
 * total_time = active_time + idle_time
 */

uint64_t CpuTicks::total() const {
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

uint64_t CpuTicks::active() const {
    return total() - idle - iowait;
}

bool readCpuTicks(CpuTicks& ticks) {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    // The first line starts with "cpu " (aggregate) -- that's what we want.
    // Individual CPU lines start with "cpu0", "cpu1", etc.
    while (std::getline(file, line)) {
        if (line.compare(0, 4, "cpu ") == 0) {
            std::istringstream iss(line);
            std::string label;
            iss >> label; // skip "cpu"
            iss >> ticks.user >> ticks.nice >> ticks.system
                >> ticks.idle >> ticks.iowait >> ticks.irq
                >> ticks.softirq >> ticks.steal;
            return true;
        }
    }
    return false;
}

double cpuUsagePercent(const CpuTicks& prev, const CpuTicks& curr) {
    uint64_t totalDelta  = curr.total()  - prev.total();
    uint64_t activeDelta = curr.active() - prev.active();

    if (totalDelta == 0) {
        return 0.0;
    }

    return (static_cast<double>(activeDelta) / static_cast<double>(totalDelta)) * 100.0;
}
