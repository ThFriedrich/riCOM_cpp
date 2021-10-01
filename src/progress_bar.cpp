#include "progress_bar.hpp"
#include <cmath>

const int ProgressBar::DEFAULT_WIDTH = 120;

ProgressBar::ProgressBar() {}

ProgressBar::ProgressBar(unsigned long n_, const char *unit_, bool b_bar_, std::ostream &out_)
{

    n = n_;
    b_bar = b_bar_;
    out = &out_;
    unit = unit_;
    unit_bar = "#";
    unit_space = "-";
}

void ProgressBar::SetStyle(const char *unit_bar_, const char *unit_space_)
{
    unit_bar = unit_bar_;
    unit_space = unit_space_;
}

int ProgressBar::GetBarLength()
{
    // get console width and according adjust the length of the progress bar
    int bar_length = static_cast<int>((DEFAULT_WIDTH - CHARACTER_WIDTH_PERCENTAGE) / 2.);
    return bar_length;
}

void ProgressBar::ClearBarField()
{
    for (int i = 0; i < DEFAULT_WIDTH; ++i)
    {
        *out << " ";
    }
    *out << "\r" << std::flush;
}

void ProgressBar::Progressed(unsigned long idx_, float print_val)
{
    try
    {
        if (idx_ > n)
            throw idx_;

        if (b_bar)
        {
            // calculate percentage of progress
            double progress_percent = idx_ * TOTAL_PERCENTAGE / n;

            // calculate the size of the progress bar
            int bar_size = GetBarLength();

            // calculate the percentage value of a unit bar
            double percent_per_unit_bar = TOTAL_PERCENTAGE / bar_size;

            // display progress bar
            *out << "\r" << "[";

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
        else
        {
            *out << std::setprecision(2) << std::fixed << print_val << std::fixed << " " << unit << "\t" << std::flush;
        }
        if (idx_ >= n)
        {
            *out << " " << std::endl << std::flush;
        }
        
    }
    catch (unsigned long e)
    {
        ClearBarField();
        std::cerr << "PROGRESS_BAR_EXCEPTION: _idx (" << e << ") went out of bounds, greater than n (" << n << ")." << std::endl
                  << std::flush;
    }
}