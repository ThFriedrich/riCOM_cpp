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
#include <thread>

#include "Ricom.h"
#include "SdlImageWindow.h"
#include "GuiUtils.h"

int run_cli(int argc, char *argv[], Ricom *ricom, CAMERA::Default_configurations &hardware_configurations)
{
    ricom->b_plot_cbed = false;
    std::string save_img = "";
    std::string save_dat = "";
    std::vector<SdlImageWindow> image_windows;
    // command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (i + 1 != argc)
        {
            // Set filename to read from .mib file
            if (strcmp(argv[i], "-filename") == 0)
            {
                ricom->camera = hardware_configurations[ricom->select_mode_by_file(argv[i + 1])];
                i++;
            }
            // Set IP of camera for TCP connection
            if (strcmp(argv[i], "-ip") == 0)
            {
                ricom->socket.ip = argv[i + 1];
                ricom->mode = RICOM::TCP;
                i++;
            }
            // Set port data-read port of camera
            if (strcmp(argv[i], "-port") == 0)
            {
                ricom->socket.port = std::stoi(argv[i + 1]);
                ricom->mode = RICOM::TCP;
                i++;
            }
            // Set width of image
            if (strcmp(argv[i], "-nx") == 0)
            {
                ricom->nx = std::stoi(argv[i + 1]);
                i++;
            }
            // Set height of image
            if (strcmp(argv[i], "-ny") == 0)
            {
                ricom->ny = std::stoi(argv[i + 1]);
                i++;
            }
            // Set width of camera
            if (strcmp(argv[i], "-cam_nx") == 0)
            {
                ricom->camera.nx_cam = std::stoi(argv[i + 1]);
                ricom->offset[0] = ((float)ricom->camera.nx_cam - 1) / 2;
                i++;
            }
            // Set height of camera
            if (strcmp(argv[i], "-cam_ny") == 0)
            {
                ricom->camera.ny_cam = std::stoi(argv[i + 1]);
                ricom->offset[1] = ((float)ricom->camera.ny_cam - 1) / 2;
                i++;
            }
            // Set skip per row
            if (strcmp(argv[i], "-skipr") == 0)
            {
                ricom->skip_row = std::stoi(argv[i + 1]);
                i++;
            }
            // Set skip per image
            if (strcmp(argv[i], "-skipi") == 0)
            {
                ricom->skip_img = std::stoi(argv[i + 1]);
                i++;
            }
            // Set kernel size
            if (strcmp(argv[i], "-k") == 0)
            {
                ricom->kernel.kernel_size = std::stoi(argv[i + 1]);
                i++;
            }
            // Set CBED Rotation
            if (strcmp(argv[i], "-r") == 0)
            {
                ricom->kernel.rotation = std::stof(argv[i + 1]);
                i++;
            }
            // Set CBED center offset
            if (strcmp(argv[i], "-offset") == 0)
            {
                ricom->offset[0] = std::stof(argv[i + 1]);
                i++;
                ricom->offset[1] = std::stof(argv[i + 1]);
                i++;
            }
            // Set CBED center offset
            if (strcmp(argv[i], "-update_offset") == 0)
            {
                ricom->update_offset = (bool)std::stoi(argv[i + 1]);
                i++;
            }
            // Set STEM radii
            if (strcmp(argv[i], "-radius") == 0)
            {
                ricom->b_vSTEM = true;
                ricom->detector.radius[0] = std::stof(argv[i + 1]);
                i++;
                ricom->detector.radius[1] = std::stof(argv[i + 1]);
                i++;
            }
            // Set depth of pixel for raw mode
            if (strcmp(argv[i], "-depth") == 0)
            {
                ricom->camera.depth = std::stoi(argv[i + 1]);
                ricom->camera.model = CAMERA::MERLIN;
                i++;
            }
            // Set number of repetitions
            if (strcmp(argv[i], "-rep") == 0)
            {
                ricom->rep = std::stoi(argv[i + 1]);
                i++;
            }
            // Set Dwell Time
            if (strcmp(argv[i], "-dwell_time") == 0)
            {
                ricom->camera.dwell_time = std::stof(argv[i + 1]);
                ricom->camera.model = CAMERA::TIMEPIX;
                i++;
            }
            // Set Number of threads
            if (strcmp(argv[i], "-threads") == 0)
            {
                ricom->n_threads = std::stoi(argv[i + 1]);
                i++;
            }
            // Set Number queue size
            if (strcmp(argv[i], "-queue_size") == 0)
            {
                ricom->queue_size = std::stoi(argv[i + 1]);
                i++;
            }
            // Set redraw interval in ms
            if (strcmp(argv[i], "-redraw_interval") == 0)
            {
                ricom->redraw_interval = std::stoi(argv[i + 1]);
                ricom->b_plot2SDL = true;
                i++;
            }
            // Set path to save reconstruction image
            if (strcmp(argv[i], "-save_img_path") == 0)
            {
                save_img = argv[i + 1];
                ricom->b_plot2SDL = true;
                i++;
            }
            // Set path to save reconstruction data
            if (strcmp(argv[i], "-save_data_path") == 0)
            {
                save_dat = argv[i + 1];
                i++;
            }
            // plot electric field
            if (strcmp(argv[i], "-plot_e_field") == 0)
            {
                ricom->b_e_mag = (bool)std::stoi(argv[i + 1]);
                i++;
            }
        }
    }

    if (ricom->b_plot2SDL)
    {
        std::thread run_thread;
        run_thread = std::thread(RICOM::run_ricom, ricom,ricom->mode);
        while (ricom->srf_ricom==NULL)
        {
            SDL_Delay(ricom->redraw_interval);
        }
        
        run_thread.detach();

        // // Initializing SDL
        SDL_DisplayMode DM;                 // To get the current display size
        SDL_Event event;                    // Event variable

        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_GetCurrentDisplayMode(0, &DM);
        float scale = (std::min)(((float)DM.w) / ricom->nx, ((float)DM.h) / ricom->ny) * 0.8;
        bool b_redraw = false;
        image_windows.push_back(SdlImageWindow("riCOM", ricom->srf_ricom, ricom->nx,ricom->ny, scale));
        if (ricom->b_vSTEM)
        {
            image_windows.push_back(SdlImageWindow("vSTEM",ricom->srf_stem, ricom->nx,ricom->ny, scale));
        }
        if (ricom->b_e_mag)
        {
            image_windows.push_back(SdlImageWindow("E-Field",ricom->srf_e_mag, ricom->nx,ricom->ny, scale));
        }

        bool b_open_window = true;
        while (b_open_window)
        {
            if (ricom->p_prog_mon != nullptr)
            {
                if (ricom->p_prog_mon->report_set_public)
                {
                    b_redraw = true;
                    ricom->p_prog_mon->report_set_public = false;
                }
                else
                {
                    b_redraw = true;
                }
            }

            if (b_redraw)
            {
                for (auto &wnd : image_windows)
                {
                    wnd.update_image();
                }
            }

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT ||
                    (event.type == SDL_WINDOWEVENT &&
                     event.window.event == SDL_WINDOWEVENT_CLOSE))
                {
                    ricom->rc_quit = true;
                    SDL_Delay(ricom->redraw_interval);
                    b_open_window = false;
                }
            }
        }
    }
    else
    {
        RICOM::run_ricom(ricom, ricom->mode);
    }
    if (save_dat != "")
    {
        save_numpy(&save_dat, ricom->nx, ricom->ny, &ricom->ricom_data);
        std::cout << "riCOM reconstruction data saved as " + save_dat << std::endl;
    }
    if (save_img != "")
    {
        save_image(&save_img, ricom->srf_ricom);
        std::cout << "riCOM reconstruction image saved as " + save_img << std::endl;
    }
    return 0;
}