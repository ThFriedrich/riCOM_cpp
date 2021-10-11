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
namespace chc = std::chrono;

#include "ricom_types.h"

#define TOTAL_PERCENTAGE 100.0
#define CHARACTER_WIDTH_PERCENTAGE 4

class ProgressMonitor
{

public:
    const static int DEFAULT_WIDTH;

    std::atomic<unsigned int> fr_count;         // frame count
    std::atomic<unsigned int> fr_count_i;       // Frame count in interval
    std::atomic<float> fr_freq;          // frame frequency
    bool report_set;        // update flag (reset externally) 
    bool first_frame;       // first frame flag
    ProgressMonitor& operator++();
    void reset_flags();
    ProgressMonitor(unsigned long fr_total, bool b_bar = true, float report_interval = 500.0, std::ostream &out = std::cerr);

private:
    bool b_bar;             // Print progress bar
    unsigned long fr_total; // Total number of frames
    std::ostream *out;      // Output stream

    std::atomic<float> fr;              // Frequncy per frame
    std::atomic<float> fr_avg;          // Average frequency
    float report_interval; // Update interval
    std::atomic<int> fr_count_a;     // Count measured points for averaging
    
    chc::time_point<chc::high_resolution_clock> time_stamp;

    const char *unit; 
    const char *unit_bar;
    const char *unit_space;

    void Report(unsigned long idx, float print_val);
    void ClearBarField();
    int GetBarLength();
};

#endif // _PROGRESS_MONITOR_