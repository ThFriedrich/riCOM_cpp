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

#include <string>
#include "Ricom.h"
#include "Camera.h"

static std::string version("0.0.4-beta");

// Forward Declarations
int run_cli(int argc, char *argv[], Ricom *ricom);
int run_gui(Ricom *ricom);
void log2file(Ricom *ricom);

# ifdef __linux__
#include <sys/utsname.h>
std::string get_os()
{
    struct utsname uts;
    uname(&uts);
    return std::string(uts.sysname) + " Kernel : " + std::string(uts.version);
}
#elif defined(_WIN32) || defined(WIN32)
    #include <stdio.h>
    #include <stdlib.h>
    #include <windows.h>

    typedef LONG NTSTATUS, *PNTSTATUS;
    #define STATUS_SUCCESS (0x00000000)

    typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    std::string get_os() {
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr) {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if ( STATUS_SUCCESS == fxPtr(&rovi) ) {
                return "Windows " + std::to_string(rovi.dwMajorVersion) + "." + std::to_string(rovi.dwMinorVersion) ;
            }
        }
    }
    RTL_OSVERSIONINFOW rovi = { 0 };
    return "Windows";
}
#else
    std::string get_os() {return "Unknown"};
#endif

int main(int argc, char *argv[])
{

    Ricom ricom;

    if (argc == 1)
    {
#ifdef _WIN32
        FreeConsole();
#endif
        log2file(&ricom);
        return run_gui(&ricom);
    }
    else
    {
        return run_cli(argc, argv, &ricom);
    }
}

// Redirect all output of cout into a log-file for GUI mode
void log2file(Ricom *ricom)
{
    if (freopen("ricom.log", "a", stdout) == NULL)
    {
        std::cout << "Error redirecting output to log file" << std::endl;
    }
    if (freopen("ricom.log", "a", stderr) == NULL)
    {
        std::cout << "Error redirecting error output to log file" << std::endl;
    }
    ricom->b_print2file = true;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::cout << std::endl;
    std::cout << "Ricom version " << version << " started at " << std::put_time(&tm, "%d/%m/%Y %H:%M:%S") << std::endl;
    std::cout << "OS: " << get_os() << std::endl;
}