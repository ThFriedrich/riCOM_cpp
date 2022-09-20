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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4067)
#pragma warning(disable : 4333)
#pragma warning(disable : 4312)
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

#include <string.h>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cfloat>
#include <ctime>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include "imfilebrowser.h"
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "imgui_ini.h"

#include "Ricom.h"
#include "fonts.h"
#include "Camera.h"
#include "MerlinInterface.h"
#include "TimepixInterface.h"

namespace chc = std::chrono;

CAMERA::Default_configurations hardware_configurations;

void run_ricom(Ricom *r, RICOM::modes mode)
{
    switch (r->camera.model)
    {
    case CAMERA::MERLIN:
        r->run_reconstruction<MerlinInterface>(mode);
        break;
    case CAMERA::TIMEPIX:
        r->run_reconstruction<TimepixInterface>(mode);
        break;
    default:
        break;
    }
}

void save_com(Ricom *ricom, std::string filename)
{

    float meta[8] = {(float)ricom->nxy * 2, (float)ricom->nx, (float)ricom->ny, 0, 0, 0, 0, 0};

    std::ofstream com_file(filename, std::ios::out | std::ios::binary);
    com_file.write(reinterpret_cast<char *>(&meta[0]), 8 * sizeof(float));
    com_file.write(reinterpret_cast<char *>(&ricom->com_map_x[0]), ricom->nxy * sizeof(float));
    com_file.write(reinterpret_cast<char *>(&ricom->com_map_y[0]), ricom->nxy * sizeof(float));
    com_file.close();
}

void run_connection_script(Ricom *ricom, MerlinSettings *merlin, std::string python_path)
{

    int m_fr_total = ((ricom->nx + ricom->skip_row) * ricom->ny + ricom->skip_img) * ricom->rep;

    std::ofstream run_script("run_script.py");
    run_script << "from merlin_interface.merlin_interface import MerlinInterface" << std::endl;
    run_script << "m = MerlinInterface(tcp_ip = \"" << ricom->socket.ip << "\" , tcp_port=" << merlin->com_port << ")" << std::endl;
    run_script << "m.hvbias = " << merlin->hvbias << std::endl;
    run_script << "m.threshold0 = " << merlin->threshold0 << std::endl;
    run_script << "m.threshold1 = " << merlin->threshold1 << std::endl;
    run_script << "m.continuousrw = " << (int)merlin->continuousrw << std::endl;
    run_script << "m.counterdepth = " << ricom->camera.depth << std::endl;
    run_script << "m.acquisitiontime = " << merlin->dwell_time << std::endl;
    run_script << "m.acquisitionperiod = " << merlin->dwell_time << std::endl;
    run_script << "m.numframestoacquire = " << m_fr_total + 1 << std::endl; // Ich verstehe nicht warum, aber er werkt.
    run_script << "m.fileenable = " << (int)merlin->save << std::endl;
    run_script << "m.runheadless = " << (int)merlin->headless << std::endl;
    run_script << "m.fileformat = " << (int)merlin->raw * 2 << std::endl;
    run_script << "m.triggerstart = " << merlin->trigger << std::endl;
    run_script << "m.startacquisition()";
    run_script.close();

    std::string command = python_path + " run_script.py";
    int r = std::system(command.c_str());
    if (r != 0)
    {
        std::cout << "main::run_connection_script: Cannot execute python run_script. Shell exited with code " << r << std::endl;
    }
}

void select_mode_by_file(const char *filename, Ricom *ricom)
{
    ricom->file_path = filename;
    if (std::filesystem::path(filename).extension() == ".t3p")
    {
        ricom->mode = RICOM::FILE;
        ricom->camera = hardware_configurations[CAMERA::TIMEPIX];
    }
    else if (std::filesystem::path(filename).extension() == ".mib")
    {
        ricom->mode = RICOM::FILE;
        ricom->camera = hardware_configurations[CAMERA::MERLIN];
    }
}

////////////////////////////////////////////////
//            GUI implementation              //
////////////////////////////////////////////////

// Vertical Splitter Container
void v_splitter(float thickness, float &size0, float &min_h, float &max_h, float offset = 0.0f)
{

    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab]);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabHovered]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive]);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().ScrollbarRounding);
    ImGui::Button("", ImVec2(-1, thickness));
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(1);
    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    }

    if (ImGui::IsItemActive())
    {
        float d = ImGui::GetMousePos().y - offset;
        if (d < min_h)
            size0 = min_h;
        else if (d > max_h)
            size0 = max_h;
        else
            size0 = d;
    }
}

int run_gui(Ricom *ricom)
{
    std::thread run_thread;
    std::thread py_thread;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_Window *window = SDL_CreateWindow("riCOM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    mINI::INIFile ini_file("imgui.ini");
    mINI::INIStructure ini_cfg;
    ini_file.read(ini_cfg);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.Fonts->AddFontFromMemoryCompressedTTF(KarlaRegular_compressed_data, KarlaRegular_compressed_size, 14.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 14.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(CousineRegular_compressed_data, CousineRegular_compressed_size, 14.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, 14.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyClean_compressed_data, ProggyClean_compressed_size, 14.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyTiny_compressed_data, ProggyTiny_compressed_size, 14.0f);

    ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
    ImVec4 clear_color = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);

    ImVec2 menu_bar_size;
    ImVec2 control_menu_size(200, 800);

    const size_t n_textures = 3;
    GLuint uiTextureIDs[n_textures];
    glGenTextures(n_textures, uiTextureIDs);

    for (size_t i = 0; i < n_textures; i++)
    {
        glBindTexture(GL_TEXTURE_2D, uiTextureIDs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // create a file browser instance
    ImGui::FileBrowser openFileDialog;
    openFileDialog.SetTitle("Open .mib or .t3p file");
    openFileDialog.SetTypeFilters({".mib", ".t3p"});
    ImGui::FileBrowser saveFileDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    saveFileDialog.SetTitle("Save image as .png");
    ImGui::FileBrowser saveDataDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    saveDataDialog.SetTitle("Save COM data in binary format");
    std::string filename = "";

    // Main loop conditional flags
    bool b_done = false;
    bool b_merlin_live_menu = true;
    bool b_acq_open = false;
    bool b_started = false;
    bool b_file_selected = false;

    // Merlin Settings (to send to the Camera)
    struct MerlinSettings merlin_settings;

    // INI file settings to restore previous session settings
    // Appearance
    int font_index = 0;
    int style_index = 0;
    ImGui_INI::check_ini_setting(ini_cfg, "Appearance", "Font", font_index);
    ImGui_INI::set_font(font_index);
    ImGui_INI::check_ini_setting(ini_cfg, "Appearance", "Style", style_index);
    ImGui_INI::set_style(style_index);
    ImGui_INI::check_ini_setting(ini_cfg, "Appearance", "Background", clear_color);
    // Hardware Settings
    std::string python_path;
#ifdef _WIN32
    python_path = "py";
#else
    python_path = "python3";
#endif
    ImGui_INI::check_ini_setting(ini_cfg, "Hardware", "Threads", ricom->n_threads);
    ImGui_INI::check_ini_setting(ini_cfg, "Hardware", "Queue Size", ricom->queue_size);
    ImGui_INI::check_ini_setting(ini_cfg, "Hardware", "Image Refresh Interval [ms]", ricom->redraw_interval);
    //Merlin Settings
    ImGui_INI::check_ini_setting(ini_cfg, "Merlin", "Live Interface Menu", b_merlin_live_menu);
    ImGui_INI::check_ini_setting(ini_cfg, "Merlin", "nx", hardware_configurations[CAMERA::MERLIN].nx_cam);
    ImGui_INI::check_ini_setting(ini_cfg, "Merlin", "ny", hardware_configurations[CAMERA::MERLIN].ny_cam);
    ImGui_INI::check_ini_setting(ini_cfg, "Merlin", "com_port", merlin_settings.com_port);
    ImGui_INI::check_ini_setting(ini_cfg, "Merlin", "data_port", ricom->socket.port);
    ImGui_INI::check_ini_setting(ini_cfg, "Merlin", "ip", ricom->socket.ip);
    ImGui_INI::check_ini_setting(ini_cfg, "Merlin", "python_path", python_path);
    // Timepix Settings
    ImGui_INI::check_ini_setting(ini_cfg, "Timepix", "nx", hardware_configurations[CAMERA::MERLIN].nx_cam);
    ImGui_INI::check_ini_setting(ini_cfg, "Timepix", "ny", hardware_configurations[CAMERA::MERLIN].ny_cam);

    const char *cmaps[] = {"Parula", "Heat", "Jet", "Turbo", "Hot", "Gray", "Magma", "Inferno", "Plasma", "Viridis", "Cividis", "Github"};
    bool b_redraw = false;
    float menu_split_v = control_menu_size.y * 0.8f;

    // Initialize Frequency approximation (for plotting only)
    ricom->kernel.approximate_frequencies((size_t)ricom->nx);

    // Main loop
    while (!b_done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                b_done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                b_done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Menu
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Appearance"))
            {
                ImGui::Combo("CBED Colormap", &ricom->cbed_cmap, cmaps, IM_ARRAYSIZE(cmaps));
                if (ImGui::ColorEdit3("Background Color", (float *)&clear_color))
                {
                    ini_cfg["Appearance"]["Background"] = ImGui_INI::ImVec2string(clear_color);
                }
                ImGui_INI::ShowFontSelector("Font", font_index, ini_cfg);
                ImGui_INI::ShowStyleSelector("Style", style_index, ini_cfg);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Hardware Settings"))
            {
                ImGui::Text("Plotting");
                if (ImGui::DragInt("Image Refresh Interval [ms]", &ricom->redraw_interval, 20, 10, 1000))
                {
                    ini_cfg["Hardware"]["Image Refresh Interval [ms]"] = std::to_string(ricom->redraw_interval);
                }
                ImGui::Separator();

                ImGui::Text("Multithreading");
                if (ImGui::DragInt("Threads", &ricom->n_threads, 1, 1, *&ricom->n_threads_max))
                {
                    ini_cfg["Hardware"]["Threads"] = std::to_string(ricom->n_threads);
                }
                if (ImGui::DragInt("Queue Size", &ricom->queue_size, 1, 1, 256))
                {
                    ini_cfg["Hardware"]["Queue Size"] = std::to_string(ricom->queue_size);
                }
                ImGui::Separator();

                ImGui::Text("Merlin Camera");
                if (ImGui::Checkbox("Live Interface Menu", &b_merlin_live_menu))
                {
                    ini_cfg["Merlin"]["Live Interface Menu"] = std::to_string(b_merlin_live_menu);
                }
                if (ImGui::DragInt("nx Merlin", &hardware_configurations[CAMERA::MERLIN].nx_cam, 1, 1, 2048))
                {
                    ini_cfg["Merlin"]["nx"] = std::to_string(hardware_configurations[CAMERA::MERLIN].nx_cam);
                }
                if (ImGui::DragInt("ny Merlin", &hardware_configurations[CAMERA::MERLIN].ny_cam, 1, 1, 2048))
                {
                    ini_cfg["Merlin"]["ny"] = std::to_string(hardware_configurations[CAMERA::MERLIN].ny_cam);
                }
                if (ImGui::InputText("IP", &ricom->socket.ip))
                {
                    ini_cfg["Merlin"]["ip"] = ricom->socket.ip;
                }
                if (ImGui::InputInt("COM-Port", &merlin_settings.com_port, 8))
                {
                    ini_cfg["Merlin"]["com_port"] = std::to_string(ricom->socket.port);
                }
                if (ImGui::InputInt("Data-Port", &ricom->socket.port, 8))
                {
                    ini_cfg["Merlin"]["data_port"] = std::to_string(ricom->socket.port);
                }
                if (ImGui::InputText("python path", &python_path))
                {
                    ini_cfg["Merlin"]["python path"] = python_path;
                }
                ImGui::Separator();

                ImGui::Text("Timepix Camera");
                // ImGui::Checkbox("Live Interface Menu", &b_timepix_live_menu);
                if (ImGui::DragInt("nx Timepix", &hardware_configurations[CAMERA::TIMEPIX].nx_cam, 1, 1, 2048))
                {
                    ini_cfg["Timepix"]["nx"] = std::to_string(hardware_configurations[CAMERA::TIMEPIX].nx_cam);
                }
                if (ImGui::DragInt("ny Timepix", &hardware_configurations[CAMERA::TIMEPIX].ny_cam, 1, 1, 2048))
                {
                    ini_cfg["Timepix"]["ny"] = std::to_string(hardware_configurations[CAMERA::TIMEPIX].ny_cam);
                }
                ImGui::EndMenu();
            }
            menu_bar_size = ImGui::GetWindowSize();
            ImGui::EndMainMenuBar();
        }

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        if (ricom->p_prog_mon != nullptr)
        {
            if (ricom->p_prog_mon->report_set_public)
            {
                b_redraw = true;
                ricom->p_prog_mon->report_set_public = false;
            }
        }
        else
        {
            b_redraw = true;
        }

        ImVec2 pos = viewport->Pos;
        pos[1] += menu_bar_size.y;
        ImGui::SetNextWindowPos(pos);

        control_menu_size.y = viewport->Size.y - menu_bar_size.y;
        ImGui::SetNextWindowSize(control_menu_size);
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));

        ImGui::Begin("Navigation", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        bool b_nx_changed = false;
        ImGui::BeginChild("Controls", ImVec2(0, menu_split_v), false);
        if (ImGui::CollapsingHeader("General Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Scan Area");
            b_nx_changed = ImGui::DragInt("nx", &ricom->nx, 1, 1, SDL_MAX_SINT32);
            ImGui::DragInt("ny", &ricom->ny, 1, 1, SDL_MAX_SINT32);
            ImGui::DragInt("Repetitions", &ricom->rep, 1, 1, SDL_MAX_SINT32);
            ImGui::BeginGroup();
            ImGui::DragInt("skip row", &ricom->skip_row, 1, 0, SDL_MAX_SINT32);
            ImGui::DragInt("skip img", &ricom->skip_img, 1, 0, SDL_MAX_SINT32);
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Adjustments for Flyback time:\n skip row: skip n frames after each scan line\n skip img: skip n frames after each scan (when using repetitions)");
            }

            ImGui::Text("CBED Centre");
            int *max_nx = (std::max)(&ricom->camera.nx_cam, &ricom->camera.ny_cam);
            bool offset_changed = ImGui::DragFloat2("Centre", &ricom->offset[0], 0.1f, 0.0, (float)*max_nx);
            if (offset_changed)
            {
                ricom->b_recompute_detector = true;
                ricom->b_recompute_kernel = true;
            }
            ImGui::Checkbox("Auto Centering", &ricom->update_offset);
            ImGui::DragFloat("Dose ( 10^ ))", &ricom->update_dose_lowbound, 0.1f, 0.0, 10.0);
        }

        if (ImGui::CollapsingHeader("RICOM Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool kernel_changed = ImGui::DragInt("Kernel Size", &ricom->kernel.kernel_size, 1, 1, 300);
            bool rot_changed = ImGui::SliderFloat("Rotation", &ricom->kernel.rotation, 0.0f, 360.0f, "%.1f deg");
            bool filter_changed = ImGui::Checkbox("Use filter?", &ricom->kernel.b_filter);
            bool filter_changed2 = ImGui::DragInt2("high / low", &ricom->kernel.kernel_filter_frequency[0], 1, 0, SDL_MAX_SINT32);
            if (rot_changed || kernel_changed || filter_changed || filter_changed2)
            {
                ricom->b_recompute_kernel = true;
            }
            if (kernel_changed || b_nx_changed)
            {
                ricom->kernel.approximate_frequencies((size_t)ricom->nx);
            }
            ImGui::PlotLines("Frequencies", ricom->kernel.f_approx.data(), ricom->kernel.f_approx.size(), 0, NULL, 0.0f, 1.0f, ImVec2(0, 50));
        }

        if (ImGui::CollapsingHeader("vSTEM Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("View vSTEM Image", &ricom->b_vSTEM);
            bool inner_changed = ImGui::SliderFloat("Inner Radius", &ricom->detector.radius[0], 0.0f, 182.0f, "%.1f px");
            bool outer_changed = ImGui::SliderFloat("Outer Radius", &ricom->detector.radius[1], 0.0f, 182.0f, "%.1f px");
            if (inner_changed || outer_changed)
            {
                ricom->b_recompute_detector = true;
            }
        }

        if (b_merlin_live_menu)
        {
            if (ImGui::CollapsingHeader("Merlin Live Mode", ImGuiTreeNodeFlags_DefaultOpen))
            {

                ImGui::InputInt("hvbias", &merlin_settings.hvbias, 1, 10);
                ImGui::InputInt("threshold0", &merlin_settings.threshold0, 1, 8);
                ImGui::InputInt("threshold1", &merlin_settings.threshold1, 1, 8);
                ImGui::InputFloat("dwell time (us)", &merlin_settings.dwell_time, 0.01, 0.1);
                ImGui::Checkbox("trigger", &merlin_settings.trigger);
                ImGui::SameLine();
                ImGui::Checkbox("headless", &merlin_settings.headless);
                ImGui::Checkbox("continuousrw", &merlin_settings.continuousrw);
                ImGui::SameLine();
                ImGui::Checkbox("raw", &merlin_settings.raw);

                if (merlin_settings.raw)
                {
                    ImGui::Text("Depth");
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::RadioButton("1", &ricom->camera.depth, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("6", &ricom->camera.depth, 6);
                    ImGui::SameLine();
                    ImGui::RadioButton("12", &ricom->camera.depth, 12);
                    ImGui::EndGroup();
                }

                ImGui::Checkbox("save file?", &merlin_settings.save);

                if (ricom->socket.b_connected)
                {
                    b_started = true;
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
                    if (ricom->socket.connection_information.size() > 0)
                    {
                        ImGui::SameLine();
                        if (ImGui::Button("Show Acqusition Info", ImVec2(-1.0f, 0.0f)))
                        {
                            b_acq_open = true;
                        }
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Connected");
                }

                if (ImGui::Button("Start Acquisition", ImVec2(-1.0f, 0.0f)))
                {
                  
                    run_thread = std::thread(run_ricom, ricom, RICOM::TCP);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    py_thread = std::thread(run_connection_script, ricom, &merlin_settings, python_path);

                    run_thread.detach();
                    py_thread.detach();
                }
            }
        }
        if (ImGui::CollapsingHeader("File reconstruction", ImGuiTreeNodeFlags_DefaultOpen))
        {

            if (ImGui::Button("Open File", ImVec2(-1.0f, 0.0f)))
            {
                openFileDialog.Open();
            }

            openFileDialog.Display();
            if (openFileDialog.HasSelected())
            {
                filename = openFileDialog.GetSelected().string();
                b_file_selected = true;
                openFileDialog.ClearSelected();
                select_mode_by_file(filename.c_str(), ricom);
            }
            if (b_file_selected)
            {
                ImGui::Text("File: %s", filename.c_str());

                if (ricom->camera.model == CAMERA::MERLIN)
                {
                    ImGui::BeginGroup();
                    ImGui::Text("Depth");
                    ImGui::RadioButton("1", &ricom->camera.depth, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("6", &ricom->camera.depth, 6);
                    ImGui::SameLine();
                    ImGui::RadioButton("12", &ricom->camera.depth, 12);
                    ImGui::EndGroup();
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Only applicable for handling recorded files, \n recorded in raw mode.");
                    }
                }
                if (ricom->camera.model == CAMERA::TIMEPIX)
                {
                    ImGui::DragInt("dwell time", &ricom->camera.dwell_time, 1, 1);
                }

                if (ImGui::Button("Run File", ImVec2(-1.0f, 0.0f)))
                {
                    run_thread = std::thread(run_ricom, ricom, RICOM::FILE);
                    b_started = true;
                    run_thread.detach();
                }
            }
        }
        ImGui::EndChild();

        float panel_h_min = control_menu_size.y * 0.4;
        float panel_h_max = control_menu_size.y - 32;
        v_splitter(5, menu_split_v, panel_h_min, panel_h_max, pos.y + ImGui::GetStyle().ItemSpacing.y * 3);

        ImGui::BeginChild("Progress", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
        if (b_started)
        {

            ImGui::ProgressBar(ricom->fr_count / (ricom->fr_total), ImVec2(-1.0f, 0.0f));
            ImGui::Text("Speed: %.2f kHz", ricom->fr_freq);
            if (ImGui::Button("Quit", ImVec2(-1.0f, 0.0f)))
            {
                ricom->rc_quit = true;
                b_redraw = true;
            }

            ImGui::Text("COM= %.2f, %.2f", ricom->com_public[0], ricom->com_public[1]);

            // CBED Plot Area
            if (b_started)
            {
                ImVec2 rem_space = ImGui::GetContentRegionAvail();
                float tex_wh = (std::min)(rem_space.x, rem_space.y);
                ImVec2 p = ImGui::GetCursorScreenPos();

                float com_rel_x = p.x + tex_wh * (ricom->com_public[0] / ricom->camera.nx_cam);
                float com_rel_y = p.y + tex_wh * (ricom->com_public[1] / ricom->camera.ny_cam);

                float centre_x = p.x + tex_wh * (ricom->offset[0] / ricom->camera.nx_cam);
                float centre_y = p.y + tex_wh * (ricom->offset[1] / ricom->camera.ny_cam);

                com_rel_x = (std::max)(p.x, com_rel_x);
                com_rel_y = (std::max)(p.y, com_rel_y);
                com_rel_x = (std::min)(p.x + tex_wh, com_rel_x);
                com_rel_y = (std::min)(p.y + tex_wh, com_rel_y);

                float cross_width = tex_wh / 15.0f;

                if (b_redraw)
                {
                    if (ricom->srf_cbed != NULL)
                    {
                        glBindTexture(GL_TEXTURE_2D, uiTextureIDs[0]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ricom->srf_cbed->w, ricom->srf_cbed->h, 0,
                                     GL_BGRA, GL_UNSIGNED_BYTE, ricom->srf_cbed->pixels);
                    }
                }
                ImGui::Image((ImTextureID)uiTextureIDs[0], ImVec2(tex_wh, tex_wh), uv_min, uv_max, tint_col, border_col);
                ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * 0.03, IM_COL32(255, 255, 255, 255), 256);
                ImGui::GetWindowDrawList()->AddLine(ImVec2(com_rel_x - cross_width, com_rel_y), ImVec2(com_rel_x + cross_width, com_rel_y), IM_COL32(255, 0, 0, 255), 1.5f);
                ImGui::GetWindowDrawList()->AddLine(ImVec2(com_rel_x, com_rel_y - cross_width), ImVec2(com_rel_x, com_rel_y + cross_width), IM_COL32(255, 0, 0, 255), 1.5f);
                if (ricom->b_vSTEM)
                {
                    ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * (ricom->detector.radius[0] / ricom->camera.nx_cam), IM_COL32(255, 50, 0, 255), 256);
                    ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * (ricom->detector.radius[1] / ricom->camera.nx_cam), IM_COL32(255, 150, 0, 255), 256);
                }
            }
        }
        ImGui::EndChild();

        control_menu_size = ImGui::GetWindowSize();
        ImGui::End();

        if (b_acq_open)
        {
            ImVec2 pos = viewport->Pos;
            pos[0] += control_menu_size[0] + 128;
            pos[1] += menu_bar_size[1] + 128;
            ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
            ImVec2 size = {200, 400};
            ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

            ImGui::Begin("Acquisition Header", &b_acq_open);
            ImGui::BeginChild("Scrolling");
            ImGui::TextWrapped(ricom->socket.connection_information.data());
            ImGui::EndChild();
            ImGui::End();
        }

        if (ricom->srf_ricom != NULL)
        {

            {
                ImVec2 pos = viewport->Pos;
                pos[0] += control_menu_size[0];
                pos[1] += menu_bar_size[1];
                ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
                ImVec2 size = {512, 512};
                ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

                ImGui::Begin("riCOM", NULL, ImGuiWindowFlags_NoScrollbar);

                if (ImGui::Button("Save Image as..."))
                {
                    saveFileDialog.Open();
                }

                ImGui::SameLine();
                if (ImGui::Button("Save COM..."))
                {
                    saveDataDialog.Open();
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1);
                if (ImGui::Combo("Colormap", &ricom->ricom_cmap, cmaps, IM_ARRAYSIZE(cmaps)))
                {
                    // if (ricom->fr_count == 0 || ricom->fr_count >= ricom->fr_total)
                    if (!ricom->b_busy)
                    {
                        ricom->draw_ricom_image();
                        b_redraw = true;
                    }
                    else
                    {
                        ricom->rescale_ricom = true;
                    }
                }

                saveFileDialog.Display();
                if (saveFileDialog.HasSelected())
                {
                    std::string img_file = saveFileDialog.GetSelected().string();
                    if (img_file.substr(img_file.size() - 4, 4) != ".png" && img_file.substr(img_file.size() - 4, 4) != ".PNG")
                    {
                        img_file += ".png";
                    }
                    saveFileDialog.ClearSelected();
                    IMG_SavePNG(ricom->srf_ricom, img_file.c_str());
                }

                saveDataDialog.Display();
                if (saveDataDialog.HasSelected())
                {
                    std::string com_file = saveDataDialog.GetSelected().string();
                    if (std::filesystem::path(com_file).extension() == "")
                    {
                        com_file += ".dat";
                    }
                    saveDataDialog.ClearSelected();
                    save_com(ricom, com_file);
                }

                ImVec2 vAvail = ImGui::GetContentRegionAvail();
                float scale = (std::min)(vAvail.x / ricom->srf_ricom->w, vAvail.y / ricom->srf_ricom->h);
                float tex_h = ricom->srf_ricom->h * scale;
                float tex_w = ricom->srf_ricom->w * scale;

                if (b_redraw)
                {
                    glBindTexture(GL_TEXTURE_2D, uiTextureIDs[1]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ricom->srf_ricom->w, ricom->srf_ricom->h, 0,
                                 GL_BGRA, GL_UNSIGNED_BYTE, ricom->srf_ricom->pixels);
                }
                ImGui::Image((ImTextureID)uiTextureIDs[1], ImVec2(tex_w, tex_h), uv_min, uv_max, tint_col, border_col);
                ImGui::End();
            }
        }

        if (ricom->srf_stem != NULL && ricom->b_vSTEM == true)
        {
            {
                ImVec2 pos = viewport->Pos;
                pos[0] += control_menu_size[0] + 256;
                pos[1] += menu_bar_size[1] + 256;
                ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
                ImVec2 size = {512, 512};
                ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

                ImGui::Begin("vSTEM", &ricom->b_vSTEM, ImGuiWindowFlags_NoScrollbar);
                if (ImGui::Button("Save Image as..."))
                {
                    saveFileDialog.Open();
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1);
                if (ImGui::Combo("Colormap", &ricom->stem_cmap, cmaps, IM_ARRAYSIZE(cmaps)))
                {
                    if (!ricom->b_busy)
                    {
                        ricom->draw_stem_image();
                        b_redraw = true;
                    }
                    else
                    {
                        ricom->rescale_stem = true;
                    }
                }

                saveFileDialog.Display();
                if (saveFileDialog.HasSelected())
                {
                    std::string img_file = saveFileDialog.GetSelected().string();
                    if (img_file.substr(img_file.size() - 4, 4) != ".png" && img_file.substr(img_file.size() - 4, 4) != ".PNG")
                    {
                        img_file += ".png";
                    }
                    saveFileDialog.ClearSelected();
                    IMG_SavePNG(ricom->srf_stem, img_file.c_str());
                }

                ImVec2 vAvail = ImGui::GetContentRegionAvail();
                float scale = (std::min)(vAvail.x / ricom->srf_stem->w, vAvail.y / ricom->srf_stem->h);
                float tex_h = ricom->srf_stem->h * scale;
                float tex_w = ricom->srf_stem->w * scale;

                if (b_redraw)
                {
                    glBindTexture(GL_TEXTURE_2D, uiTextureIDs[2]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ricom->srf_stem->w, ricom->srf_stem->h, 0,
                                 GL_BGRA, GL_UNSIGNED_BYTE, ricom->srf_stem->pixels);
                }

                ImGui::Image((ImTextureID)uiTextureIDs[2], ImVec2(tex_w, tex_h), uv_min, uv_max, tint_col, border_col);
                ImGui::End();
            }
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        b_redraw = false;
    }

    if (run_thread.joinable())
        run_thread.join();
    if (py_thread.joinable())
        py_thread.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    ini_file.write(ini_cfg, true);
    return 0;
}

////////////////////////////////////////////////
//            CLI implementation              //
////////////////////////////////////////////////

void update_image(SDL_Texture *tex, SDL_Renderer *renderer, SDL_Surface *srf)
{
    if (SDL_UpdateTexture(tex, NULL, srf->pixels, srf->pitch) == -1)
    {
        std::cout << "SDL_UpdateTexture failed: " << SDL_GetError() << std::endl;
    }
    if (SDL_RenderCopy(renderer, tex, NULL, NULL) == -1)
    {
        std::cout << "SDL_RenderCopy failed: " << SDL_GetError() << std::endl;
    }
    SDL_RenderPresent(renderer);
}

int run_cli(int argc, char *argv[], Ricom *ricom)
{
    // command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (i + 1 != argc)
        {
            // Set filename to read from .mib file
            if (strcmp(argv[i], "-filename") == 0)
            {
                select_mode_by_file(argv[i + 1], ricom);
                i++;
            }
            if (strcmp(argv[i], "-ip") == 0)
            {
                ricom->socket.ip = argv[i + 1];
                ricom->mode = RICOM::TCP;
                i++;
            }
            // Set port to connect to Merlin
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
            if (strcmp(argv[i], "-time") == 0)
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
                i++;
            }
        }
    }

    // Initializing SDL
    SDL_Window *window = NULL;          // Pointer for the window
    SDL_Window *window_stem = NULL;     // Pointer for the window
    SDL_Renderer *renderer = NULL;      // Pointer for the renderer
    SDL_Renderer *renderer_stem = NULL; // Pointer for the renderer
    SDL_Texture *tex = NULL;            // Texture for the window;
    SDL_Texture *stem = NULL;           // Texture for the window;
    SDL_DisplayMode DM;                 // To get the current display size
    SDL_Event event;                    // Event variable

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GetCurrentDisplayMode(0, &DM);
    float scale = std::min(((float)DM.w) / ricom->nx, ((float)DM.h) / ricom->ny) * 0.8;
    bool b_redraw = false;
    
    // Creating window
    window = SDL_CreateWindow("riCOM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scale * ricom->nx, scale * ricom->ny, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    // Creating Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    // Creating texture for hardware rendering
    tex = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_STATIC, ricom->nx, ricom->ny);
    if (tex == NULL)
    {
        std::cout << "Texture could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    // Maintain Pixel aspect ratio on resizing
    if (SDL_RenderSetLogicalSize(renderer, scale * ricom->nx, scale * ricom->ny) < 0)
    {
        std::cout << "Logical size could not be set! SDL Error: " << SDL_GetError() << std::endl;
    }
    if (ricom->b_vSTEM)
    {
        // Creating window
        window_stem = SDL_CreateWindow("vSTEM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scale * ricom->nx, scale * ricom->ny, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
        if (window_stem == NULL)
        {
            std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        }
        // Creating Renderer
        renderer_stem = SDL_CreateRenderer(window_stem, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer_stem == NULL)
        {
            std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        }
        // Creating texture for hardware rendering
        stem = SDL_CreateTexture(renderer_stem, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_STATIC, ricom->nx, ricom->ny);
        if (stem == NULL)
        {
            std::cout << "Texture could not be created! SDL Error: " << SDL_GetError() << std::endl;
        }
        // Maintain Pixel aspect ratio on resizing
        if (SDL_RenderSetLogicalSize(renderer_stem, scale * ricom->nx, scale * ricom->ny) < 0)
        {
            std::cout << "Logical size could not be set! SDL Error: " << SDL_GetError() << std::endl;
        }
    }
    std::thread run_thread;
    run_thread = std::thread(run_ricom, ricom, ricom->mode);
    run_thread.detach();

    SDL_Delay(ricom->redraw_interval * 2);

    while (1)
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
            update_image(tex, renderer, ricom->srf_ricom);
            if (ricom->b_vSTEM)
            {
                update_image(stem, renderer_stem, ricom->srf_stem);
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
                return 0;
            }
        }
    }
    return 0;
}

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

int main(int argc, char *argv[])
{

    Ricom ricom;
    Ricom *ricom_ptr = &ricom;

    if (argc == 1)
    {
        #ifdef _WIN32
        FreeConsole();
        #endif
        log2file(ricom_ptr);
        return run_gui(ricom_ptr);
    }
    else
    {
        return run_cli(argc, argv, ricom_ptr);
    }
}
