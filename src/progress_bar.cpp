#include "progress_bar.hpp"
#include <cmath>

const int ProgressBar::DEFAULT_WIDTH = 100;

ProgressBar::ProgressBar() {}

ProgressBar::ProgressBar(unsigned long n_, const char *description_, std::ostream &out_)
{

    n = n_;

    description = description_;
    out = &out_;

    unit_bar = "#";
    unit_space = "-";
    desc_width = std::strlen(description); // character width of description field
}

void ProgressBar::SetStyle(const char *unit_bar_, const char *unit_space_)
{

    unit_bar = unit_bar_;
    unit_space = unit_space_;
}


int ProgressBar::GetBarLength()
{

    // get console width and according adjust the length of the progress bar

    int bar_length = static_cast<int>((DEFAULT_WIDTH - desc_width - CHARACTER_WIDTH_PERCENTAGE) / 2.);

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

void ProgressBar::Progressed(unsigned long idx_)
{
    try
    {
        if (idx_ > n)
            throw idx_;

        // calculate the size of the progress bar
        int bar_size = GetBarLength();

        // calculate percentage of progress
        double progress_percent = idx_ * TOTAL_PERCENTAGE / n;

        // calculate the percentage value of a unit bar
        double percent_per_unit_bar = TOTAL_PERCENTAGE / bar_size;

        // display progress bar
        *out << " " << description << " [";

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

        *out << "]" << std::setw(CHARACTER_WIDTH_PERCENTAGE + 1) << std::setprecision(1) << std::fixed << progress_percent << "%\r" << std::flush;
    }
    catch (unsigned long e)
    {
        ClearBarField();
        std::cerr << "PROGRESS_BAR_EXCEPTION: _idx (" << e << ") went out of bounds, greater than n (" << n << ")." << std::endl
                  << std::flush;
    }
}
void ProgressBar::Progressed(unsigned long idx_, float print_val, std::string const &unit)
{
    try
    {
        if (idx_ > n)
            throw idx_;

        // calculate the size of the progress bar
        int bar_size = GetBarLength();

        // calculate percentage of progress
        double progress_percent = idx_ * TOTAL_PERCENTAGE / n;

        // calculate the percentage value of a unit bar
        double percent_per_unit_bar = TOTAL_PERCENTAGE / bar_size;

        // display progress bar
        *out << " " << description << " [";

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
        *out << std::setprecision(2) << std::fixed << print_val << std::fixed << " " << unit << "\r" << std::flush;
    }
    catch (unsigned long e)
    {
        ClearBarField();
        std::cerr << "PROGRESS_BAR_EXCEPTION: _idx (" << e << ") went out of bounds, greater than n (" << n << ")." << std::endl
                  << std::flush;
    }
}
