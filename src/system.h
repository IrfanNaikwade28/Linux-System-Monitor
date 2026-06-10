#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>

// Reads system uptime from /proc/uptime.
// The file contains a single line: "<uptime_seconds> <idle_seconds>"
// Returns uptime in seconds, or -1 on failure.
double readUptime();

// Formats seconds into a human-readable string like "2h 34m" or "3d 5h".
std::string formatUptime(double seconds);

#endif
