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

#include "Ricom.h"
#include "Camera.h"

// Forward Declarations
int run_cli(int argc, char *argv[], Ricom *ricom, CAMERA::Default_configurations &hardware_configurations);
int run_gui(Ricom *ricom, CAMERA::Default_configurations &hardware_configurations);
void log2file(Ricom *ricom);

int main(int argc, char *argv[])
{

    Ricom ricom;
    Ricom *ricom_ptr = &ricom;
    CAMERA::Default_configurations hardware_configurations;

    if (argc == 1)
    {
#ifdef _WIN32
        FreeConsole();
#endif
        log2file(ricom_ptr);
        return run_gui(ricom_ptr, hardware_configurations);
    }
    else
    {
        return run_cli(argc, argv, ricom_ptr, hardware_configurations);
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
    time_t timetoday;
    time(&timetoday);
    std::cout << std::endl
              << "##########################################################################" << std::endl;
    std::cout << "              Ricom started at " << asctime(localtime(&timetoday));
    std::cout << "##########################################################################" << std::endl;
}