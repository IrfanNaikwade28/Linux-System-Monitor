#include "system.h"
#include <fstream>
#include <sstream>
#include <cstdio>

/*
 * /proc/uptime contains a single line with two numbers:
 *
 *   12345.67 98765.43
 *
 * The first number is total uptime in seconds (including sleep time).
 * The second is idle time across all CPUs (not useful for us).
 *
 * We read just the first number.
 */

double readUptime() {
    std::ifstream file("/proc/uptime");
    if (!file.is_open()) {
        return -1.0;
    }

    double uptime = 0.0;
    file >> uptime;
    return uptime;
}

std::string formatUptime(double seconds) {
    if (seconds < 0) {
        return "unknown";
    }

    int totalSeconds = static_cast<int>(seconds);
    int days    = totalSeconds / 86400;
    int hours   = (totalSeconds % 86400) / 3600;
    int minutes = (totalSeconds % 3600) / 60;

    char buf[64];

    if (days > 0) {
        std::snprintf(buf, sizeof(buf), "%dd %dh %dm", days, hours, minutes);
    } else if (hours > 0) {
        std::snprintf(buf, sizeof(buf), "%dh %dm", hours, minutes);
    } else {
        std::snprintf(buf, sizeof(buf), "%dm", minutes);
    }

    return std::string(buf);
}
