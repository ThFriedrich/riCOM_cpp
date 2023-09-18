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

#include "ProgressMonitor.h"

ProgressMonitor::ProgressMonitor(size_t fr_total, bool b_bar, float report_interval, std::ostream &out) : fr_count(0), fr_count_i(0), fr_freq(0),
                                                                                                          report_set(false), report_set_public(false),
                                                                                                          first_frame(true), fr(0), fr_avg(0), fr_count_a(0),
                                                                                                          time_stamp(chc::high_resolution_clock::now()), unit("kHz"),
                                                                                                          unit_bar("#"), unit_space("-")
{
    this->fr_total = fr_total;
    this->b_bar = b_bar;
    this->out = &out;
    this->report_interval = report_interval;
}

int ProgressMonitor::GetBarLength()
{
    // get console width and according adjust the length of the progress bar
    int bar_length = static_cast<int>((TERMINAL_WIDTH - CHARACTER_WIDTH_PERCENTAGE) / 2.);
    return bar_length;
}

void ProgressMonitor::ClearBarField()
{
    for (int i = 0; i < TERMINAL_WIDTH; ++i)
    {
        *out << " ";
    }
    *out << "\r" << std::flush;
}

ProgressMonitor &ProgressMonitor::operator++()
{
    fr_count_i++;
    fr_count++;
    auto mil_secs = chc::duration_cast<float_ms>(chc::high_resolution_clock::now() - time_stamp).count();
    if (mil_secs > report_interval || fr_count >= fr_total)
    {
        fr = fr_count_i / mil_secs;
        fr_avg += fr;
        fr_count_a++;
        fr_freq = fr_avg / fr_count_a;
        Report(fr_count, fr_avg / fr_count_a);
        report_set = true;
        report_set_public = true;
    }
    return *this;
}

ProgressMonitor &ProgressMonitor::operator+=(int step)
{
    fr_count_i += step;
    fr_count += step;
    auto mil_secs = chc::duration_cast<float_ms>(chc::high_resolution_clock::now() - time_stamp).count();
    if (mil_secs > report_interval || fr_count >= fr_total)
    {
        fr = fr_count_i / mil_secs;
        fr_avg += fr;
        fr_count_a++;
        fr_freq = fr_avg / fr_count_a;
        Report(fr_count, fr_avg / fr_count_a);
        report_set = true;
        report_set_public = true;
    }
    return *this;
}


void ProgressMonitor::reset_flags()
{
    fr_count_i = 0;
    report_set = false;
    time_stamp = chc::high_resolution_clock::now();
}

void ProgressMonitor::Report(size_t idx, float print_val)
{
    try
    {
        if (idx > fr_total)
            throw idx;

        if (b_bar) // Print out the Progressbar and Frequency
        {
            // calculate percentage of progress
            double progress_percent = idx * TOTAL_PERCENTAGE / fr_total;

            // calculate the size of the progress bar
            int bar_size = GetBarLength();

            // calculate the percentage value of a unit bar
            double percent_per_unit_bar = TOTAL_PERCENTAGE / bar_size;

            // display progress bar
            *out << "\r"
                 << "[";

            for (int bar_length = 0; bar_length <= bar_size - 1; ++bar_length)
            {
                if (bar_length * percent_per_unit_bar < progress_percent)
                {
                    *out << unit_bar;
                }
                else
                {
                    *out << unit_space;
                }
            }
            *out << "]" << std::setw(CHARACTER_WIDTH_PERCENTAGE + 1);
            *out << std::setprecision(2) << std::fixed << print_val << std::fixed << " " << unit << std::flush;
        }
        if (idx >= fr_total)
        {
            *out << " " << std::endl
                 << std::flush;
        }
    }
    catch (size_t e)
    {
        ClearBarField();
        std::cerr << "EXCEPTION: frame index (" << e << ") went out of bounds (fr_total = " << fr_total << ")." << std::endl
                  << std::flush;
    }
}