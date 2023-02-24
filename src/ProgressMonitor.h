/* Copyright (C) 2021 Thomas Friedrich, Chu-Ping Yu, 
 * University of Antwerp - All Rights Reserved. 
 * You may use, distribute and modify
 * this code under the terms of the GPL3 license.
 * You should have received a copy of the GPL3 license with
 * this file. If not, please visit: 
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 * 
 * Authors: 
 *   Thomas Friedrich <thomas.friedrich@uantwerpen.be>
 *   Chu-Ping Yu <chu-ping.yu@uantwerpen.be>
 */

#ifndef _PROGRESS_MONITOR_
#define _PROGRESS_MONITOR_

#ifndef _WIN32
#include <sys/ioctl.h>
#endif

#include <iostream>
#include <iomanip>
#include <cstring>
#include <atomic>
#include <chrono>
#include <cmath>

namespace chc = std::chrono;
typedef chc::duration<float, std::milli> float_ms;

#define TOTAL_PERCENTAGE 100.0
#define CHARACTER_WIDTH_PERCENTAGE 4
#define TERMINAL_WIDTH 120
class ProgressMonitor
{

public:
    const static int DEFAULT_WIDTH;

    std::atomic<size_t> fr_count;   // frame count
    std::atomic<size_t> fr_count_i; // Frame count in interval
    float fr_freq;                        // frame frequency
    std::atomic<bool> report_set;         // update flag (reset internally)
    std::atomic<bool> report_set_public;  // update flag (reset externally)
    bool first_frame;                     // first frame flag
    ProgressMonitor &operator++();
    void reset_flags();
    explicit ProgressMonitor(size_t fr_total, bool b_bar = true, float report_interval = 250.0, std::ostream &out = std::cerr);

private:
    bool b_bar;             // Print progress bar
    size_t fr_total; // Total number of frames
    std::ostream *out;      // Output stream

    float fr;                    // Frequncy per frame
    float fr_avg;                // Average frequency
    float report_interval;       // Update interval
    std::atomic<int> fr_count_a; // Count measured points for averaging

    chc::time_point<chc::high_resolution_clock> time_stamp;

    const char *unit;
    const char *unit_bar;
    const char *unit_space;

    void Report(unsigned long idx, float print_val);
    void ClearBarField();
    int GetBarLength();
};

#endif // _PROGRESS_MONITOR_
