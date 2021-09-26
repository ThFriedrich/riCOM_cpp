#include <string.h>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cfloat>
#include <ctime>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include "imfilebrowser.h"

#include "Ricom.h"
#include "fonts.h"
#include "ricom_types.h"

namespace chc = std::chrono;

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning(disable : 4067)
#pragma warning(disable : 4333)
#pragma warning(disable : 4312)
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void run_file(Ricom *r)
{
    r->reset();
    r->mode = RICOM::modes::FILE;
    r->run();
}

void run_live(Ricom *r)
{
    r->reset();
    r->mode = RICOM::modes::LIVE;
    r->run();
}

#ifdef _WIN32
void run_fake_merlin()
{
    std::system("/py fake_merlin.py");
}
#else
void run_fake_merlin()
{
    int r = std::system("python3 fake_merlin.py");
    if (r != 0)
    {
        std::cout << "Error running fake_merlin.py" << std::endl;
    }
}
#endif

#ifdef _WIN32
void run_connection_script()
{
    // std::filesystem::path temp_path = std::filesystem::temp_directory_path();
    // std::filesystem::path file = "m_list.txt";
    // std::string file_directory = (temp_path / file).string();
    // int r = std::system( ("py " + file_directory).c_str() );
    int r = std::system("py m_list.py");
    if (r != 0)
    {
        std::cout << "Cannot find m_list, generate file first." << std::endl;
    }
}
#else
void run_connection_script()
{
    // std::filesystem::path temp_path = std::filesystem::temp_directory_path();
    // std::filesystem::path file = "m_list.txt";
    // std::string file_directory = (temp_path / file).string();
    // int r = std::system( ("python3 " + file_directory).c_str() );
    int r = std::system("python3 m_list.py");
    if (r != 0)
    {
        std::cout << "Cannot find m_list, generate file first." << std::endl;
    }
}
#endif

void select_mode_by_file(const char *filename, Ricom *ricom)
{
    if (std::filesystem::path(filename).extension() == ".t3p")
    {
        ricom->mode = RICOM::FILE;
        ricom->t3p_path = filename;
        ricom->detector_type = RICOM::TIMEPIX;
    }
    else if (std::filesystem::path(filename).extension() == ".mib")
    {
        ricom->mode = RICOM::FILE;
        ricom->mib_path = filename;
        ricom->detector_type = RICOM::MERLIN;
    }
}

int run_gui(Ricom *ricom)
{
    std::thread t1;
    std::thread t2;
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

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();

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
    // Main loop
    bool done = false;
    bool b_merlin_list = false;
    bool b_acq_open = false;
    bool b_running = false;
    bool file_selected = false;

    int c_port = 6341;
    char ip[16] = "127.0.0.1";

    std::string filename = "";

    auto start_perf = chc::high_resolution_clock::now();
    typedef std::chrono::duration<float, std::milli> double_ms;

    // Merlin Parameter;
    int m_threshold0 = 0;
    int m_threshold1 = 511;
    float m_dwell_time = 0.1; // unit ms
    bool m_save = false;
    bool m_trigger = true; // false: internal, true: rising edge
    bool m_headless = true;
    bool m_raw = true;

    const char *cmaps[] = {"Parula", "Heat", "Jet", "Turbo", "Hot", "Gray", "Magma", "Inferno", "Plasma", "Viridis", "Cividis", "Github"};
    bool b_redraw = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
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
                ImGui::ColorEdit3("Background Color", (float *)&clear_color);
                ShowFontSelector("Font");
                ImGui::ShowStyleSelector("Style");
                ImGui::Separator();
                if (ImGui::TreeNode("Style Editor"))
                {
                    ImGui::ShowStyleEditor();
                    ImGui::TreePop();
                }
                ImGui::EndMenu();
            }
            menu_bar_size = ImGui::GetWindowSize();
            ImGui::EndMainMenuBar();
        }

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        auto mil_secs = std::chrono::duration_cast<double_ms>(chc::high_resolution_clock::now() - start_perf).count();
        if (mil_secs > 350.0)
        {
            b_redraw = true;
        }

        {
            ImVec2 pos = viewport->Pos;
            pos[1] += menu_bar_size.y;
            ImGui::SetNextWindowPos(pos);

            ImVec2 size = viewport->Size;
            size[0] = control_menu_size[0];
            ImGui::SetNextWindowSize(size);
            ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));

            ImGui::Begin("Navigation", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

            if (ImGui::CollapsingHeader("General Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Scan Area");
                ImGui::DragInt("nx", &ricom->nx, 1, 1);
                ImGui::DragInt("ny", &ricom->ny, 1, 1);
                ImGui::DragInt("Repetitions", &ricom->rep, 1, 1);

                ImGui::Text("CBED Centre");
                bool offset_changed = ImGui::DragFloat2("Centre", &ricom->offset[0], 0.1f, 0.0, 256.0);
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
                if (rot_changed || kernel_changed)
                {
                    ricom->b_recompute_kernel = true;
                }
            }

            if (ImGui::CollapsingHeader("vSTEM Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("View vSTEM Image", &ricom->use_detector);
                bool inner_changed = ImGui::SliderFloat("Inner Radius", &ricom->detector.radius[0], 0.0f, 182.0f, "%.1f px");
                bool outer_changed = ImGui::SliderFloat("Outer Radius", &ricom->detector.radius[1], 0.0f, 182.0f, "%.1f px");
                if (inner_changed || outer_changed)
                {
                    ricom->b_recompute_detector = true;
                }
            }

            if (ImGui::CollapsingHeader("Merlin Live Mode", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::InputText("IP", ip, sizeof(ip)))
                {
                    ricom->ip = ip;
                }
                ImGui::InputInt("COM-Port", &c_port, 8);
                ImGui::InputInt("Data-Port", &ricom->port, 8);

                ImGui::Text("Scan Engine Settings");
                ImGui::DragInt("skip row", &ricom->skip_row, 1, 1);
                ImGui::DragInt("skip img", &ricom->skip_img, 1, 1);

                if (ImGui::Button("Merlin Setup", ImVec2(-1.0f, 0.0f)))
                {
                    b_merlin_list = true;
                }

                if (ricom->b_connected)
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
                    if (ricom->acq_header.size() > 0)
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
                    t1 = std::thread(run_live, ricom);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    t2 = std::thread(run_connection_script);
                    b_running = true;
                    t1.detach();
                    t2.detach();
                }
            }

            if (ImGui::CollapsingHeader(".mib file reconstruction", ImGuiTreeNodeFlags_DefaultOpen))
            {

                if (ImGui::Button("Open File", ImVec2(-1.0f, 0.0f)))
                {
                    openFileDialog.Open();
                }

                openFileDialog.Display();
                if (openFileDialog.HasSelected())
                {
                    filename = openFileDialog.GetSelected().string();
                    file_selected = true;
                    openFileDialog.ClearSelected();
                    select_mode_by_file(filename.c_str(), ricom);
                }
                if (file_selected)
                {
                    ImGui::Text("File: %s", filename.c_str());
                    ImGui::BeginGroup();
                    ImGui::Text("Depth");
                    ImGui::RadioButton("1", &ricom->depth, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("6", &ricom->depth, 6);
                    ImGui::SameLine();
                    ImGui::RadioButton("12", &ricom->depth, 12);
                    ImGui::EndGroup();
                    ImGui::DragInt("dwell time (.t3p)", &ricom->dwell_time, 1, 1);
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Only applicable for handling recorded files, \n recorded in raw mode.");
                    }
                    if (ImGui::Button("Run File", ImVec2(-1.0f, 0.0f)))
                    {
                        t1 = std::thread(run_file, ricom);
                        b_running = true;
                        t1.detach();
                    }
                }
            }
            ImGui::Separator();
            ImGui::BeginChild("Progress", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar);

            if (b_running)
            {
                ImGui::ProgressBar(ricom->fr_count_total / (ricom->fr_total), ImVec2(-1.0f, 0.0f));
                ImGui::Text("Speed: %.2f kHz", ricom->fr_freq);
                if (ImGui::Button("Quit", ImVec2(-1.0f, 0.0f)))
                {
                    ricom->rc_quit = true;
                    b_redraw = true;
                }

                ImGui::Text("COM= %.2f, %.2f", ricom->com_public[0], ricom->com_public[1]);

                // CBED Plot Area
                ImGui::BeginChild("CBED", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
                ImVec2 rem_space = ImGui::GetContentRegionAvail();
                float tex_wh = (std::min)(rem_space.x, (rem_space.y - ImGui::GetStyle().WindowPadding.y * 3.0f));

                ImVec2 p = ImGui::GetCursorScreenPos();

                float com_rel_x = p.x + tex_wh * (ricom->com_public[0] / ricom->nx_merlin);
                float com_rel_y = p.y + tex_wh * (ricom->com_public[1] / ricom->ny_merlin);

                float centre_x = p.x + tex_wh * (ricom->offset[0] / ricom->nx_merlin);
                float centre_y = p.y + tex_wh * (ricom->offset[1] / ricom->ny_merlin);

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
                if (ricom->use_detector)
                {
                    ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * (ricom->detector.radius[0] / ricom->nx_merlin), IM_COL32(255, 50, 0, 255), 256);
                    ImGui::GetWindowDrawList()->AddCircle(ImVec2(centre_x, centre_y), tex_wh * (ricom->detector.radius[1] / ricom->nx_merlin), IM_COL32(255, 150, 0, 255), 256);
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
            control_menu_size = ImGui::GetWindowSize();
            ImGui::End();
        }

        if (b_merlin_list)
        {
            ImVec2 pos = viewport->Pos;
            pos[0] += control_menu_size[0] + 128;
            pos[1] += menu_bar_size[1] + 128;
            ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
            ImVec2 size = {200, 400};
            ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

            ImGui::Begin("Merlin Setup List", &b_merlin_list);
            ImGui::BeginChild("Scrolling");

            ImGui::InputInt("threshold0", &m_threshold0, 8);
            ImGui::InputInt("threshold1", &m_threshold1, 8);
            ImGui::InputFloat("dwell time (us)", &m_dwell_time, 64);
            ImGui::Checkbox("trigger", &m_trigger);
            ImGui::Checkbox("headless", &m_headless);
            ImGui::Checkbox("raw", &m_raw);

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::RadioButton("1", &ricom->depth, 1);
            ImGui::SameLine();
            ImGui::RadioButton("6", &ricom->depth, 6);
            ImGui::SameLine();
            ImGui::RadioButton("12", &ricom->depth, 12);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::Text("Depth");

            ImGui::Checkbox("save file?", &m_save);

            if (ImGui::Button("Confirm"))
            {
                // std::filesystem::path temp_path = std::filesystem::temp_directory_path();
                // std::filesystem::path file = "m_list.txt";
                // std::ofstream m_list (temp_path / file);
                int fr_total = ((ricom->nx + ricom->skip_row) * ricom->ny + ricom->skip_img) * ricom->rep;

                std::ofstream m_list("m_list.py");
                m_list << "from merlin_interface.merlin_interface import MerlinInterface" << '\n';
                m_list << "m = MerlinInterface(tcp_ip = \"" << ip << "\" , tcp_port=" << c_port << ")" << '\n';
                m_list << "m.hvbias = 120" << '\n';
                m_list << "m.threshold0 = " << m_threshold0 << '\n';
                m_list << "m.threshold1 = " << m_threshold1 << '\n';
                m_list << "m.continuousrw = 1" << '\n';
                m_list << "m.counterdepth = " << ricom->depth << '\n';
                m_list << "m.acquisitiontime = " << m_dwell_time << '\n';
                m_list << "m.acquisitionperiod = " << m_dwell_time << '\n';
                m_list << "m.numframestoacquire = " << fr_total+1 << '\n'; // Ich verstehe nicht warum, aber er werkt.
                m_list << "m.fileenable = " << (int)m_save << '\n';
                m_list << "m.runheadless = " << (int)m_headless << '\n';
                m_list << "m.fileformat = " << (int)m_raw * 2 << '\n';
                m_list << "m.triggerstart = " << m_trigger << '\n';
                m_list << "m.startacquisition()";
                m_list.close();
            }
            ImGui::EndChild();
            ImGui::End();
        }

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
            ImGui::TextWrapped(ricom->acq.data());
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

                if (ImGui::Combo("Colormap", &ricom->ricom_cmap, cmaps, IM_ARRAYSIZE(cmaps)))
                {
                    if (ricom->fr_count_total == 0)
                    {
                        ricom->rescale_ricom_image();
                    }
                    else
                    {
                        ricom->rescale_ricom = true;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Save Image as..."))
                {
                    saveFileDialog.Open();
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

                ImVec2 vMin = ImGui::GetWindowContentRegionMin();
                ImVec2 vMax = ImGui::GetWindowContentRegionMax();

                float my_tex_w = vMax.x - vMin.x;
                float my_tex_h = vMax.y - vMin.y;
                float scale = (std::min)(my_tex_w / ricom->srf_ricom->w, my_tex_h / ricom->srf_ricom->h);
                my_tex_h = ricom->srf_ricom->h * scale;
                my_tex_w = ricom->srf_ricom->w * scale;

                if (b_redraw)
                {
                    glBindTexture(GL_TEXTURE_2D, uiTextureIDs[1]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ricom->srf_ricom->w, ricom->srf_ricom->h, 0,
                                 GL_BGRA, GL_UNSIGNED_BYTE, ricom->srf_ricom->pixels);
                }
                ImGui::Image((ImTextureID)uiTextureIDs[1], ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
                ImGui::End();
            }
        }

        if (ricom->srf_stem != NULL && ricom->use_detector == true)
        {
            {
                ImVec2 pos = viewport->Pos;
                pos[0] += control_menu_size[0] + 256;
                pos[1] += menu_bar_size[1] + 256;
                ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
                ImVec2 size = {512, 512};
                ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

                ImGui::Begin("vSTEM", &ricom->use_detector, ImGuiWindowFlags_NoScrollbar);

                
                
                if (ImGui::Combo("Colormap", &ricom->stem_cmap, cmaps, IM_ARRAYSIZE(cmaps)))
                {
                    if (ricom->fr_count_total == 0)
                    {
                        ricom->rescale_stem_image();
                    }
                    else
                    {
                        ricom->rescale_stem = true;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Save Image as..."))
                {
                    saveFileDialog.Open();
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
                ImVec2 vMin = ImGui::GetWindowContentRegionMin();
                ImVec2 vMax = ImGui::GetWindowContentRegionMax();

                float my_tex_w = vMax.x - vMin.x;
                float my_tex_h = vMax.y - vMin.y;
                float scale = (std::min)(my_tex_w / ricom->srf_stem->w, my_tex_h / ricom->srf_stem->h);
                my_tex_h = ricom->srf_stem->h * scale;
                my_tex_w = ricom->srf_stem->w * scale;

                if (b_redraw)
                {
                    glBindTexture(GL_TEXTURE_2D, uiTextureIDs[2]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ricom->srf_stem->w, ricom->srf_stem->h, 0,
                                 GL_BGRA, GL_UNSIGNED_BYTE, ricom->srf_stem->pixels);
                }

                ImGui::Image((ImTextureID)uiTextureIDs[2], ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
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

        if (b_redraw)
        {
            b_redraw = false;
            start_perf = chc::high_resolution_clock::now();
        }
    }

    //Cleanup
    // if (ricom->srf_ricom != NULL)
    // SDL_FreeSurface(ricom->srf_ricom);
    // if (ricom->srf_stem != NULL)
    // SDL_FreeSurface(ricom->srf_stem);

    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
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
                ricom->ip = argv[i + 1];
                ricom->mode = RICOM::LIVE;
                i++;
            }
            // Set port to connect to Merlin
            if (strcmp(argv[i], "-port") == 0)
            {
                ricom->port = std::stoi(argv[i + 1]);
                ricom->mode = RICOM::modes::LIVE;
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
                ricom->detector.offset[0] = std::stof(argv[i + 1]);
                i++;
                ricom->offset[1] = std::stof(argv[i + 1]);
                ricom->detector.offset[1] = std::stof(argv[i + 1]);
                i++;
            }
            // Set STEM radii
            if (strcmp(argv[i], "-radius") == 0)
            {
                ricom->use_detector = true;
                ricom->detector.radius[0] = std::stof(argv[i + 1]);
                i++;
                ricom->detector.radius[1] = std::stof(argv[i + 1]);
                i++;
            }
            // Set depth of pixel for raw mode
            if (strcmp(argv[i], "-depth") == 0)
            {
                ricom->depth = std::stoi(argv[i + 1]);
                ricom->detector_type = RICOM::MERLIN;
                i++;
            }
            if (strcmp(argv[i], "-rep") == 0)
            {
                ricom->rep = std::stoi(argv[i + 1]);
                i++;
            }
            // Set Dwell Time
            if (strcmp(argv[i], "-time") == 0)
            {
                ricom->dwell_time = std::stof(argv[i + 1]);
                ricom->detector_type = RICOM::TIMEPIX;
                i++;
            }
        }
    }
    if (ricom->mode == RICOM::LIVE)
    {
        run_live(ricom);
    }
    else if (ricom->mode == RICOM::FILE)
    {
        run_file(ricom);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (freopen("ricom.log", "a", stdout) == NULL)
    {
        std::cout << "Error redirecting output to log file" << std::endl;
    }
    if (freopen("ricom.log", "a", stderr) == NULL)
    {
        std::cout << "Error redirecting error output to log file" << std::endl;
    }

    time_t timetoday;
    time(&timetoday);
    std::cout << std::endl
              << "##########################################################################" << std::endl;
    std::cout << "              Ricom started at " << asctime(localtime(&timetoday));
    std::cout << "##########################################################################" << std::endl;

    Ricom ricom;
    Ricom* ricom_ptr = &ricom;
    if (argc == 1)
    {
        return run_gui(ricom_ptr);
    }
    else
    {
        return run_cli(argc, argv, ricom_ptr);
    }
}
