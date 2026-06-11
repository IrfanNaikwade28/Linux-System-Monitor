#include "memory.h"
#include <fstream>
#include <sstream>

/*
 * /proc/meminfo contains key-value pairs like:
 *
 *   MemTotal:        7843452 kB
 *   MemFree:          234567 kB
 *   MemAvailable:    3123456 kB
 *   ...
 *
 * We read "MemTotal" and "MemAvailable" directly.
 * Used memory = MemTotal - MemAvailable.
 * This matches the behavior of the `free` command.
 *
 * Note: "MemAvailable" is an estimate provided by the kernel of how much
 * memory is available for starting new applications, without swapping.
 * It accounts for reusable memory (page cache, buffers, etc.).
 */

static uint64_t parseMemField(const std::string& line) {
    // Each line is like "MemTotal:     7843452 kB"
    std::istringstream iss(line);
    std::string label;
    uint64_t value = 0;
    iss >> label >> value;
    return value;
}

bool readMemoryInfo(MemoryInfo& info) {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        return false;
    }

    bool foundTotal     = false;
    bool foundAvailable = false;
    std::string line;

    while (std::getline(file, line)) {
        if (line.compare(0, 9, "MemTotal:") == 0) {
            info.totalKb = parseMemField(line);
            foundTotal = true;
        } else if (line.compare(0, 13, "MemAvailable:") == 0) {
            info.availableKb = parseMemField(line);
            foundAvailable = true;
        }
        if (foundTotal && foundAvailable) {
            break;
        }
    }

    if (!foundTotal || !foundAvailable) {
        return false;
    }

    info.usedKb      = info.totalKb - info.availableKb;
    info.usedPercent = (static_cast<double>(info.usedKb) /
                        static_cast<double>(info.totalKb)) * 100.0;
    return true;
}

std::string formatBytes(uint64_t kb) {
    // Convert KB to GB or MB for display
    double gb = static_cast<double>(kb) / (1024.0 * 1024.0);
    if (gb >= 1.0) {
        // Format to 1 decimal place
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f GB", gb);
        return std::string(buf);
    }
    double mb = static_cast<double>(kb) / 1024.0;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.0f MB", mb);
    return std::string(buf);
}
