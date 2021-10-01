#ifndef _PROGRESS_BAR_
#define _PROGRESS_BAR_

#ifndef _WIN32
    #include <sys/ioctl.h>
#endif

#include <iostream>
#include <iomanip>
#include <cstring>

#define TOTAL_PERCENTAGE 100.0
#define CHARACTER_WIDTH_PERCENTAGE 4

class ProgressBar{

public: 

    const static int DEFAULT_WIDTH;

    ProgressBar();
    ProgressBar(unsigned long n_, const char *unit_="", bool b_bar_=true, std::ostream& out_=std::cerr);

    void SetStyle(const char* unit_bar_, const char* unit_space_);		

    void Progressed(unsigned long idx_, float print_val);

private:
	
    bool b_bar;
    unsigned long n;
    unsigned int desc_width;
    std::ostream* out;
		
    const char *unit;
    const char *unit_bar;
    const char *unit_space;
		
    void ClearBarField();
    int GetBarLength();

};

#endif
