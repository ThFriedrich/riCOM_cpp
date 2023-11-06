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

#include <string>
#include <thread>
#include <iostream>
#include <complex>
#include <map>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

#include "imfilebrowser.h"
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "ImGuiINI.hpp"

#include "Ricom.h"
#include "fonts.h"
#include "GuiUtils.h"
#include "ImGuiImageWindow.h"

// Macros
#define GENERIC_WINDOW(name) (generic_windows_f.find(name)->second)
#define GENERIC_WINDOW_C(name) (generic_windows_c.find(name)->second)

// Forward Declarations
template <typename T>
inline void update_views(std::map<std::string, ImGuiImageWindow<T>> &generic_windows_f, Ricom *ricom, bool b_restarted, bool trigger, bool b_redraw);
inline void bind_tex(SDL_Surface *srf, GLuint tex_id);

////////////////////////////////////////////////
//            GUI implementation              //
////////////////////////////////////////////////

int run_gui(Ricom *ricom)
{
    std::thread run_thread;
    std::thread py_thread;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) // fail here
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // if (SDL_Init(SDL_INIT_TIMER) != 0) // fail here
    // {
    //     printf("Error: %s\n", SDL_GetError());
    //     return -1;
    // }    
    // if (SDL_Init(SDL_INIT_AUDIO) != 0) // fail here
    // {
    //     printf("Error: %s\n", SDL_GetError());
    //     return -1;
    // }    
    // if (SDL_Init(SDL_INIT_VIDEO) != 0) // fail here
    // {
    //     printf("Error: %s\n", SDL_GetError());
    //     return -1;
    // }    
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0) // fail here
    // {
    //     printf("Error: %s\n", SDL_GetError());
    //     return -1;
    // }    
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0) // fail here
    // {
    //     printf("Error: %s\n", SDL_GetError());
    //     return -1;
    // }    
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0) // fail here
    // {
    //     printf("Error: %s\n", SDL_GetError());
    //     return -1;
    // }    
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0) // fail here
    // {
    //     printf("Error: %s\n", SDL_GetError());
    //     return -1;
    // }


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

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // Multi-Viewport not yet supported with SDL2
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;// Enable Multi-Viewport / Platform Windows

    ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

    ImVec2 menu_bar_size;
    ImVec2 control_menu_size(200, 800);

    const size_t n_textures = 16;
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

    // create a file browser instances
    ImGui::FileBrowser openFileDialog;
    openFileDialog.SetTitle("Open .mib or .t3p file");
    openFileDialog.SetTypeFilters({".mib", ".t3p", ".tpx3"});
    ImGui::FileBrowser saveFileDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    saveFileDialog.SetTitle("Save image as .png");
    ImGui::FileBrowser saveDataDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    saveDataDialog.SetTitle("Save COM data in binary format");
    std::string filename = "";

    // Main loop conditional flags
    bool b_done = false;
    bool b_acq_open = false;
    bool b_started = false;
    bool b_file_selected = false;
    bool b_restarted = false;

    // Cheetah Setting (to send to the server)
    CheetahComm cheetah_comm;
    ricom->socket.connect_socket();


    // INI file settings to restore previous session settings
    // Appearance
    int font_index = 0;
    int style_index = 0;
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "CBED Colormap", ricom->cbed_cmap);
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "Font", font_index);
    ImGuiINI::set_font(font_index);
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "Style", style_index);
    ImGuiINI::set_style(style_index);
    // Hardware Settings
    std::string python_path;
#ifdef _WIN32
    python_path = "py";
#else
    python_path = "python3";
#endif
    ImGuiINI::check_ini_setting(ini_cfg, "Hardware", "Threads", ricom->n_threads);
    ImGuiINI::check_ini_setting(ini_cfg, "Hardware", "Queue Size", ricom->queue_size);
    ImGuiINI::check_ini_setting(ini_cfg, "Hardware", "Image Refresh Interval [ms]", ricom->redraw_interval);
    // Advapix Settings
    ImGuiINI::check_ini_setting(ini_cfg, "Advapix", "nx/y", ricom->n_cam);
    // Cheetah Settings
    ImGuiINI::check_ini_setting(ini_cfg, "Cheetah", "nx/y", ricom->n_cam);
    ImGuiINI::check_ini_setting(ini_cfg, "Cheetah", "data_port", ricom->socket.port);
    ImGuiINI::check_ini_setting(ini_cfg, "Cheetah", "ip", ricom->socket.ip);

    const char *cmaps[] = {"Parula", "Heat", "Jet", "Turbo", "Hot", "Gray", "Magma", "Inferno", "Plasma", "Viridis", "Cividis", "Github", "HSV"};
    bool b_redraw = false;
    bool b_trigger_update = false;
    float menu_split_v = control_menu_size.y * 0.8f;

    // Initialize Frequency approximation (for plotting only)
    ricom->kernel.approximate_frequencies((size_t)ricom->nx);

    // Initialize all possible Windows, so everything is on the stack
    bool show_com_x = false;
    bool show_com_y = false;
    bool ricom_fft = false;
    bool vstem_fft = false;
    bool e_field_fft = false;
    GIM_Flags common_flags = GIM_Flags::SaveImButton | GIM_Flags::SaveDataButton | GIM_Flags::ColormapSelector | GIM_Flags::PowerSlider;
    std::map<std::string, ImGuiImageWindow<float>> generic_windows_f;
    std::map<std::string, ImGuiImageWindow<std::complex<float>>> generic_windows_c;
    generic_windows_f.emplace("RICOM", ImGuiImageWindow<float>("RICOM", &uiTextureIDs[1], true, 9, common_flags | GIM_Flags::FftButton));
    generic_windows_f.emplace("RICOM-FFT", ImGuiImageWindow<float>("RICOM-FFT", &uiTextureIDs[2], false, 4, common_flags, &ricom_fft));
    GENERIC_WINDOW("RICOM").fft_window = &GENERIC_WINDOW("RICOM-FFT");

    generic_windows_f.emplace("vSTEM", ImGuiImageWindow<float>("vSTEM", &uiTextureIDs[3], true, 9, common_flags | GIM_Flags::FftButton, &ricom->b_vSTEM));
    generic_windows_f.emplace("vSTEM-FFT", ImGuiImageWindow<float>("vSTEM-FFT", &uiTextureIDs[4], false, 4, common_flags, &vstem_fft));
    GENERIC_WINDOW("vSTEM").fft_window = &GENERIC_WINDOW("vSTEM-FFT");

    generic_windows_f.emplace("COM-X", ImGuiImageWindow<float>("RICOM-COMX", &uiTextureIDs[5], true, 9, common_flags, &show_com_x));
    generic_windows_f.emplace("COM-Y", ImGuiImageWindow<float>("RICOM-COMY", &uiTextureIDs[6], true, 9, common_flags, &show_com_y));

    generic_windows_c.emplace("E-FIELD", ImGuiImageWindow<std::complex<float>>("E-Field", &uiTextureIDs[7], true, 12, common_flags | GIM_Flags::FftButton, &ricom->b_e_mag));
    generic_windows_f.emplace("E-Field-FFT", ImGuiImageWindow<float>("E-Field-FFT", &uiTextureIDs[8], false, 4, common_flags, &e_field_fft));
    GENERIC_WINDOW_C("E-FIELD").fft_window = &GENERIC_WINDOW("E-Field-FFT");

    ricom->kernel.draw_surfaces();
    bind_tex(ricom->kernel.srf_kx, uiTextureIDs[9]);
    bind_tex(ricom->kernel.srf_ky, uiTextureIDs[10]);

    ImGuiID dock_id = 3775;
    Main_Dock main_dock(dock_id);
    static int drag_min_pos = 1;

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
                ImGuiINI::ShowFontSelector("Font", font_index, ini_cfg);
                ImGuiINI::ShowStyleSelector("Style", style_index, ini_cfg);
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
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Imaging Modes"))
            {
                if (ImGui::Checkbox("Show riCOM", &ricom->b_ricom))
                {
                    if (&ricom->b_ricom)
                    {
                        GENERIC_WINDOW("RICOM").set_data(ricom->nx, ricom->ny, &ricom->ricom_data);
                    }
                    else
                    {
                        *GENERIC_WINDOW("RICOM").pb_open = false;
                    }
                }
                if (ImGui::Checkbox("Show CoM-X", &show_com_x))
                {
                    if (show_com_x)
                    {
                        GENERIC_WINDOW("COM-X").set_data(ricom->nx, ricom->ny, &ricom->comx_data);
                    }
                    else
                    {
                        *GENERIC_WINDOW("COM-X").pb_open = false;
                    }
                }
                if (ImGui::Checkbox("Show CoM-Y", &show_com_y))
                {
                    if (show_com_y)
                    {
                        GENERIC_WINDOW("COM-Y").set_data(ricom->nx, ricom->ny, &ricom->comy_data);
                    }
                    else
                    {
                        *GENERIC_WINDOW("COM-Y").pb_open = false;
                    }
                }
                if (ImGui::Checkbox("Show E-Field", &ricom->b_e_mag))
                {
                    if (ricom->b_e_mag)
                    {
                        GENERIC_WINDOW_C("E-FIELD").set_data(ricom->nx, ricom->ny, &ricom->e_field_data);
                    }
                    else
                    {
                        *GENERIC_WINDOW_C("E-FIELD").pb_open = false;
                    }
                }
                if (ImGui::Checkbox("View vSTEM Image", &ricom->b_vSTEM))
                {
                    if (ricom->b_vSTEM)
                    {
                        GENERIC_WINDOW("vSTEM").set_data(ricom->nx, ricom->ny, &ricom->stem_data);
                    }
                    else
                    {
                        *GENERIC_WINDOW("vSTEM").pb_open = false;
                    }
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
            int *max_nx = &ricom->n_cam;
            bool offset_changed = ImGui::DragFloat2("Centre", &ricom->offset[0], 0.1f, 0.0, (float)*max_nx);
            if (offset_changed)
            {
                ricom->b_recompute_detector = true;
                ricom->b_recompute_kernel = true;
            }
            ImGui::Checkbox("Auto Centering", &ricom->update_offset);
        }

        if (ImGui::CollapsingHeader("RICOM Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static int filter_max = 8;
            static bool init_kernel_img = true;
            bool kernel_changed = ImGui::DragInt("Kernel Size", &ricom->kernel.kernel_size, 1, 1, 300);
            if (kernel_changed)
                filter_max = ceil(sqrt(pow(ricom->kernel.kernel_size, 2) * 2));
            bool rot_changed = ImGui::SliderFloat("Rotation", &ricom->kernel.rotation, 0.0f, 360.0f, "%.1f deg");
            bool filter_changed = ImGui::Checkbox("Use filter?", &ricom->kernel.b_filter);
            bool filter_changed2 = ImGui::DragInt2("low / high", &ricom->kernel.kernel_filter_frequency[0], 1, 0, filter_max);
            if (init_kernel_img || rot_changed || kernel_changed || filter_changed || filter_changed2)
            {
                init_kernel_img = false;
                if (ricom->b_busy)
                {
                    ricom->b_recompute_kernel = true;
                }
                else
                {
                    ricom->kernel.compute_kernel();
                }
                GENERIC_WINDOW("RICOM").reset_min_max();
                ricom->kernel.draw_surfaces();
                bind_tex(ricom->kernel.srf_kx, uiTextureIDs[9]);
                bind_tex(ricom->kernel.srf_ky, uiTextureIDs[10]);
            }
            if (kernel_changed || b_nx_changed)
            {
                ricom->kernel.approximate_frequencies((size_t)ricom->nx);
            }
            float sxk = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.y * 3) / 2;
            ImGui::Image((void *)(intptr_t)uiTextureIDs[9], ImVec2(sxk, sxk), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
            ImGui::SameLine();
            ImGui::Image((void *)(intptr_t)uiTextureIDs[10], ImVec2(sxk, sxk), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
            ImGui::PlotLines("Frequencies", ricom->kernel.f_approx.data(), ricom->kernel.f_approx.size(), 0, NULL, 0.0f, 1.0f, ImVec2(0, 50));
        }
        if (ricom->b_vSTEM)
        {
            if (ImGui::CollapsingHeader("vSTEM Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool inner_changed = ImGui::SliderFloat("Inner Radius", &ricom->detector.radius[0], 0.0f, 182.0f, "%.1f px");
                bool outer_changed = ImGui::SliderFloat("Outer Radius", &ricom->detector.radius[1], 0.0f, 182.0f, "%.1f px");
                if (inner_changed || outer_changed)
                {
                    ricom->b_recompute_detector = true;
                    GENERIC_WINDOW("vSTEM").reset_min_max();
                }
            }
        }

        if (ImGui::CollapsingHeader("Stream reconstruction", ImGuiTreeNodeFlags_DefaultOpen)){

            if (ImGui::Button("Preview", ImVec2(-1.0f, 0.0f)))
            {
                cheetah_comm.stop();
                ricom->b_continuous = true;
                cheetah_comm.tpx3_det_config();
                cheetah_comm.tpx3_cam_init();
                cheetah_comm.tpx3_destination();


                // ricom->socket.connect_socket();
                // ricom->socket.flush_socket();
                b_started = true;
                b_restarted = true;
                ricom->camera = RICOM::CHEETAH;
                run_thread = std::thread(&Ricom::run, ricom, 1);
                run_thread.detach();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                cheetah_comm.start();
                // RICOM::run(ricom, RICOM::TCP);
                // GENERIC_WINDOW("RICOM").set_data(ricom->nx, ricom->ny, &ricom->ricom_data);
            }

            if (ImGui::Button("Acquire", ImVec2(-1.0f, 0.0f)))
            {
                cheetah_comm.stop();
                ricom->b_continuous = false;
                cheetah_comm.tpx3_det_config();
                cheetah_comm.tpx3_cam_init();
                cheetah_comm.tpx3_destination();
                ricom->camera = RICOM::CHEETAH;
                run_thread = std::thread(&Ricom::run, ricom, 1);
                run_thread.detach();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                cheetah_comm.start();
                // GENERIC_WINDOW("RICOM").set_data(ricom->nx, ricom->ny, &ricom->ricom_data);
            }

            if (ImGui::Button("Stop", ImVec2(-1.0f, 0.0f)))
            {
                cheetah_comm.stop();
                ricom->rc_quit = true;
                ricom->b_continuous = false;
                // b_redraw = true;
            }

            ImGui::Checkbox("Cumulative Mode", &ricom->b_cumulative);

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
                ricom->mode = 0;
                ricom->file_path = filename;
                if (std::filesystem::path(filename).extension() == ".t3p") 
                {
                    ricom->camera = RICOM::ADVAPIX;
                    ImGui::DragInt("dwell time", &ricom->dt, 1, 1);
                }
                else if (std::filesystem::path(filename).extension() == ".tpx3") 
                {ricom->camera = RICOM::CHEETAH;}

            }
            if (b_file_selected)
            {
                ImGui::Text("File: %s", filename.c_str());

                if (ImGui::Button("Run File", ImVec2(-1.0f, 0.0f)))
                {
                    run_thread = std::thread(&Ricom::run, ricom, 0);
                    b_started = true;
                    b_restarted = true;
                    run_thread.detach();
                    // GENERIC_WINDOW("RICOM").set_data(ricom->nx, ricom->ny, &ricom->ricom_data);
                }
            }
        }
        ImGui::EndChild();

        float panel_h_min = control_menu_size.y * 0.4;
        float panel_h_max = control_menu_size.y - 32;
        v_splitter(5, menu_split_v, panel_h_min, panel_h_max, pos.y + ImGui::GetStyle().ItemSpacing.y * 3);

        ImGui::BeginChild("Progress", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
        ImGui::ProgressBar(ricom->fr_count / (ricom->fr_total), ImVec2(-1.0f, 0.0f));
        ImGui::Text("Speed: %.2f kHz", ricom->fr_freq);
        if (ImGui::Button("Quit", ImVec2(-1.0f, 0.0f)))
        {
            ricom->rc_quit = true;
            b_redraw = true;
        }

        ImGui::Text("COM= %.2f, %.2f", ricom->com_public[0], ricom->com_public[1]);

        // CBED Plot Area/DockSpace
        ImGui::DockSpace(1319);
        static auto first_time = true;
        if (first_time)
        {
            first_time = false;
            ImGui::DockBuilderRemoveNode(1319); // clear any previous layout
            ImGui::DockBuilderAddNode(1319, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderDockWindow("CBED", 1319);
            ImGui::DockBuilderFinish(1319);
        }
        ImGui::EndChild();
        control_menu_size = ImGui::GetWindowSize();
        ImGui::End();

        main_dock.render(ImVec2(control_menu_size.x, pos.y), ImVec2(viewport->Size.x - control_menu_size.x, viewport->Size.y - menu_bar_size.y));

        ImGui::Begin("CBED", nullptr, ImGuiWindowFlags_NoScrollbar);
        ImGui::Checkbox("Plot CBED", &ricom->b_plot_cbed);
        ImVec2 rem_space = ImGui::GetContentRegionAvail();
        float tex_wh = (std::min)(rem_space.x, rem_space.y);
        ImVec2 p = ImGui::GetCursorScreenPos();

        float com_rel_x = p.x + tex_wh * (ricom->com_public[0] / ricom->n_cam);
        float com_rel_y = p.y + tex_wh * (ricom->com_public[1] / ricom->n_cam);

        float centre_x = p.x + tex_wh * (ricom->offset[0] / ricom->n_cam);
        float centre_y = p.y + tex_wh * (ricom->offset[1] / ricom->n_cam);

        com_rel_x = (std::max)(p.x, com_rel_x);
        com_rel_y = (std::max)(p.y, com_rel_y);
        com_rel_x = (std::min)(p.x + tex_wh, com_rel_x);
        com_rel_y = (std::min)(p.y + tex_wh, com_rel_y);

        float cross_width = tex_wh / 15.0f;
        if (b_redraw)
        {
            if (ricom->srf_cbed != NULL)
            {
                bind_tex(ricom->srf_cbed, uiTextureIDs[0]);
            }
        }
        ImGui::Image((ImTextureID)uiTextureIDs[0], ImVec2(tex_wh, tex_wh), uv_min, uv_max, tint_col, border_col);
        ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * 0.03, IM_COL32(255, 255, 255, 255), 256);
        ImGui::GetWindowDrawList()->AddLine(ImVec2(com_rel_x - cross_width, com_rel_y), ImVec2(com_rel_x + cross_width, com_rel_y), IM_COL32(255, 0, 0, 255), 1.5f);
        ImGui::GetWindowDrawList()->AddLine(ImVec2(com_rel_x, com_rel_y - cross_width), ImVec2(com_rel_x, com_rel_y + cross_width), IM_COL32(255, 0, 0, 255), 1.5f);
        if (ricom->b_vSTEM)
        {
            ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * (ricom->detector.radius[0] / ricom->n_cam), IM_COL32(255, 50, 0, 255), 256);
            ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * (ricom->detector.radius[1] / ricom->n_cam), IM_COL32(255, 150, 0, 255), 256);
        }
        ImGui::End();

        if (b_acq_open)
        {
            ImVec2 pos_t = viewport->Pos;
            pos_t[0] += control_menu_size[0] + 128;
            pos_t[1] += menu_bar_size[1] + 128;
            ImGui::SetNextWindowPos(pos_t, ImGuiCond_FirstUseEver);
            ImVec2 size = {200, 400};
            ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

            ImGui::Begin("Acquisition Header", &b_acq_open);
            ImGui::BeginChild("Scrolling");
            ImGui::TextWrapped(ricom->socket.connection_information.data());
            ImGui::EndChild();
            ImGui::End();
        }

        // Render all generic Image Windows
        bool trigger = (b_trigger_update != ricom->b_busy) && (ricom->b_busy == false);
        bool redraw_tick = (ricom->b_busy && b_redraw && b_started);
        update_views(generic_windows_f, ricom, b_restarted, trigger, redraw_tick);
        update_views(generic_windows_c, ricom, b_restarted, trigger, redraw_tick);

        b_restarted = false;
        b_trigger_update = ricom->b_busy;

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

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

template <typename T>
void update_views(std::map<std::string, ImGuiImageWindow<T>> &generic_windows_f, Ricom *ricom, bool b_restarted, bool trigger, bool b_redraw)
{
    for (auto &wnd : generic_windows_f)
    {
        if (wnd.first == "RICOM")
        {
            if (b_restarted)
            {
                wnd.second.set_nx_ny(ricom->nx, ricom->ny);
            }
            wnd.second.render_window(b_redraw, ricom->fr_count, ricom->kernel.kernel_size, trigger);
        }
        else
        {
            if (*wnd.second.pb_open)
            {
                if (b_restarted)
                {
                    wnd.second.set_nx_ny(ricom->nx, ricom->ny);
                }
                wnd.second.render_window(b_redraw, ricom->fr_count, trigger);
            }
        }
    }
}

void bind_tex(SDL_Surface *srf, GLuint tex_id)
{
    glBindTexture(GL_TEXTURE_2D, (tex_id));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, srf->w, srf->h, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, srf->pixels);
}