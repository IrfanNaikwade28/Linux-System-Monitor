/*
 * Linux System Monitor
 *
 * A simple terminal-based system resource monitor that reads live data
 * from the Linux /proc filesystem and POSIX APIs.
 *
 * Refreshes every 1 second. No external dependencies.
 */

#include "cpu.h"
#include "memory.h"
#include "process.h"
#include "disk.h"
#include "system.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <vector>

// Clears the terminal and moves cursor to top-left.
// Uses ANSI escape codes -- works on virtually all Linux terminals.
static void clearScreen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

static void printSeparator() {
    std::cout << "----------------------------\n";
}

static void printHeader() {
    std::cout << "Linux System Monitor\n";
    printSeparator();
}

int main() {
    CpuTicks prevTicks, currTicks;

    // Take the first CPU snapshot so the next read has something to diff against.
    if (!readCpuTicks(prevTicks)) {
        std::cerr << "Error: Cannot read /proc/stat. Are you on Linux?\n";
        return 1;
    }

    while (true) {
        // --- Read all system data ---
        readCpuTicks(currTicks);
        double cpuPct = cpuUsagePercent(prevTicks, currTicks);
        prevTicks = currTicks;

        MemoryInfo mem;
        readMemoryInfo(mem);

        DiskInfo disk;
        readDiskInfo(disk);

        double uptime = readUptime();
        int procCount = countProcesses();
        std::vector<ProcessInfo> topProcs = scanProcesses();

        // --- Display ---
        clearScreen();
        printHeader();

        // CPU
        std::cout << "CPU Usage      : " << std::fixed << std::setprecision(1)
                  << cpuPct << "%\n";

        // Memory
        std::cout << "Memory Usage   : " << formatBytes(mem.usedKb)
                  << " / " << formatBytes(mem.totalKb)
                  << " (" << std::setprecision(0) << mem.usedPercent << "%)\n";

        // Disk
        double diskTotalGb = static_cast<double>(disk.totalBytes) / (1024.0 * 1024.0 * 1024.0);
        double diskUsedGb  = static_cast<double>(disk.usedBytes)  / (1024.0 * 1024.0 * 1024.0);
        std::cout << "Disk Usage     : " << std::setprecision(1) << diskUsedGb
                  << " GB / " << diskTotalGb << " GB"
                  << " (" << std::setprecision(0) << disk.usedPercent << "%)\n";

        // Uptime
        std::cout << "System Uptime  : " << formatUptime(uptime) << "\n";

        // Process count
        std::cout << "Processes      : " << procCount << "\n";

        // Top processes by memory
        printSeparator();
        std::cout << "Top Processes (by memory)\n";
        std::cout << std::left
                  << std::setw(8)  << "PID"
                  << std::setw(20) << "NAME"
                  << std::setw(10) << "MEMORY"
                  << "\n";

        int displayCount = (static_cast<int>(topProcs.size() < 8)
                            ? static_cast<int>(topProcs.size()) : 8);
        for (int i = 0; i < displayCount; i++) {
            const auto& p = topProcs[i];
            std::cout << std::setw(8)  << p.pid
                      << std::setw(20) << p.name
                      << std::setw(10) << formatBytes(p.rssKb)
                      << "\n";
        }

        // Sleep 1 second before next refresh
        sleep(1);
    }

    return 0;
}
