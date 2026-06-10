#ifndef PROCESS_H
#define PROCESS_H

#include <cstdint>
#include <string>
#include <vector>

struct ProcessInfo {
    int         pid  = 0;
    std::string name;
    uint64_t    rssKb = 0;  // resident set size in KB from /proc/[pid]/statm
};

// Scans /proc for all process directories (numeric entries).
// For each PID, reads the process name from /proc/[pid]/comm
// and RSS from /proc/[pid]/statm.
//
// Processes that disappear mid-scan (deleted/terminated) are silently skipped.
// This is normal on a live Linux system -- PIDs come and go constantly.
//
// Returns a vector of ProcessInfo, sorted by memory usage descending.
std::vector<ProcessInfo> scanProcesses();

// Returns the total number of processes (all /proc/[pid] directories).
int countProcesses();

#endif
