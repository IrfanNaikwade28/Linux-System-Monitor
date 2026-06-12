#include "process.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <cctype>

/*
 * Linux exposes process information under /proc/[pid]/ where [pid] is a
 * numeric directory. Key files we read:
 *
 *   /proc/[pid]/comm   - The command name (executable name, max 15 chars).
 *   /proc/[pid]/statm  - Memory usage in pages. Fields:
 *                         size  resident  shared  text  lib  data  dt
 *                         We read "resident" (field 2) and multiply by page size
 *                         (typically 4096 bytes = 4 pages per KB).
 *
 * Why processes can disappear during scanning:
 *   Between reading the directory listing and reading a specific PID's files,
 *   a process may exit. The kernel removes the /proc/[pid] directory when a
 *   process terminates. This is completely normal -- it is NOT an error.
 *   We simply skip any PID whose files we cannot read.
 *
 * Page size: Linux page size is almost always 4096 bytes. We define it here
 * as a constant for converting between pages and kilobytes.
 */

static const long PAGE_SIZE_KB = 4;  // 4096 bytes = 4 KB

static bool isNumeric(const std::string& s) {
    for (char c : s) {
        if (!std::isdigit(c)) return false;
    }
    return !s.empty();
}

std::vector<ProcessInfo> scanProcesses() {
    std::vector<ProcessInfo> processes;

    DIR* procDir = opendir("/proc");
    if (!procDir) {
        return processes;
    }

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        // Only look at numeric directory names (PIDs)
        std::string name(entry->d_name);
        if (!isNumeric(name)) {
            continue;
        }

        int pid = std::stoi(name);

        // Read process name from /proc/[pid]/comm
        std::string commPath = "/proc/" + name + "/comm";
        std::string processName;
        std::ifstream commFile(commPath);
        if (commFile.is_open()) {
            std::getline(commFile, processName);
            // comm may have a trailing newline
            if (!processName.empty() && processName.back() == '\n') {
                processName.pop_back();
            }
        } else {
            // Process disappeared -- skip it
            continue;
        }

        // Read RSS from /proc/[pid]/statm
        // Format: size resident shared text lib data dt
        // We want "resident" (second field) in pages
        std::string statmPath = "/proc/" + name + "/statm";
        std::ifstream statmFile(statmPath);
        uint64_t rssPages = 0;
        if (statmFile.is_open()) {
            uint64_t size = 0, resident = 0;
            statmFile >> size >> resident;
            rssPages = resident;
        } else {
            // Process disappeared -- skip it
            continue;
        }

        ProcessInfo info;
        info.pid   = pid;
        info.name  = processName;
        info.rssKb = rssPages * PAGE_SIZE_KB;
        processes.push_back(info);
    }

    closedir(procDir);

    // Sort by memory usage, highest first
    std::sort(processes.begin(), processes.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) {
                  return a.rssKb > b.rssKb;
              });

    return processes;
}

int countProcesses() {
    int count = 0;
    DIR* procDir = opendir("/proc");
    if (!procDir) {
        return 0;
    }

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (isNumeric(std::string(entry->d_name))) {
            count++;
        }
    }

    closedir(procDir);
    return count;
}
