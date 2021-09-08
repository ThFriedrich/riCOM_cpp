#include <string.h>
#include <thread>
#include <chrono>
#include <stdlib.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "imfilebrowser.h"

#include "Ricom.h"
#include "fonts.h"

namespace chc = std::chrono;

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4067)
#pragma warning (disable: 4333)
#pragma warning (disable: 4312)
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void run_file(Ricom *r, std::string path, size_t nx, size_t ny)
{
    r->reset();
    r->init(path);
    r->run(nx, ny);
}

void run_live(Ricom *r, size_t nx, size_t ny)
{
    r->reset();
    r->run(nx, ny);
}

#ifdef _WIN32
void run_fake_merlin()
{
    std::system("py fake_merlin.py");
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

int main(int, char **)
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
    ImGui::FileBrowser fileDialog;

    // (optional) set browser properties
    fileDialog.SetTitle("Open .mib file");
    fileDialog.SetTypeFilters({".mib"});

    Ricom *ricom = new Ricom();

    // Main loop
    bool done = false;
    bool b_connected = false;
    bool b_acq_open = false;
    bool b_running = false;

    static int d_port = 6342;
    static int c_port = 6341;
    static char ip[16] = "127.0.0.1";

    static int width = 257;
    static int height = 256;

    static std::string filename = "";

    auto start_perf = chc::high_resolution_clock::now();
    typedef std::chrono::duration<float, std::milli> double_ms;

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
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Placeholder: Save Settings");
                ImGui::MenuItem("Placeholder: Save Image");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Appearance"))
            {
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
                ImGui::DragInt("nx", &width);
                ImGui::DragInt("ny", &height);
                ImGui::DragInt("Repetitions", &ricom->rep,1,1);

                ImGui::Text("CBED corrections");
                bool rot_changed = ImGui::SliderFloat("Rotation", &ricom->kernel.rotation, 0.0f, 360.0f, "%.1f deg");
                bool offset_changed = ImGui::DragFloat2("Centre", &ricom->offset[0], 0.1f, 0.0, 256.0);
                if (offset_changed)
                {
                    ricom->b_recompute_detector = true;
                    ricom->b_recompute_kernel = true;
                }
                if (rot_changed)
                {
                    ricom->b_recompute_kernel = true;
                }
                ImGui::Checkbox("Auto Update Center", &ricom->update_offset);
                ImGui::DragInt("Update Dose Lower Bound ( 10^ ))", &ricom->update_dose_lowbound, 0.1f, 0.0, 10);
                ImGui::BeginGroup();
                ImGui::Text("Depth");
                ImGui::RadioButton("1", &ricom->depth, 1);
                ImGui::SameLine();
                ImGui::RadioButton("6", &ricom->depth, 6);
                ImGui::SameLine();
                ImGui::RadioButton("12", &ricom->depth, 12);
                ImGui::EndGroup();
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Only applicable for handling recorded files, \n recorded in raw mode.");
                }
            }

            if (ImGui::CollapsingHeader("RICOM Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool kernel_changed = ImGui::DragInt("Kernel Size", &ricom->kernel.kernel_size, 1, 1, 300);
                if (kernel_changed)
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
                ImGui::InputText("IP", ip, 16);
                ImGui::InputInt("COM-Port", &c_port, 8);
                ImGui::InputInt("Data-Port", &d_port, 8);

                if (ImGui::Button("Connect to Merlin", ImVec2(-1.0f, 0.0f)))
                {
                    t2 = std::thread(run_fake_merlin);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    ricom->init(ip, d_port);
                    b_connected = true;
                }

                if (b_connected)
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
                    t1 = std::thread(run_live, ricom, width, height);
                    b_running = true;
                    t1.detach();
                }
            }

            if (ImGui::CollapsingHeader(".mib file reconstruction", ImGuiTreeNodeFlags_DefaultOpen))
            {

                if (ImGui::Button("Open File", ImVec2(-1.0f, 0.0f)))
                {
                    fileDialog.Open();
                }

                fileDialog.Display();
                if (fileDialog.HasSelected())
                {
                    filename = fileDialog.GetSelected().string();
                    fileDialog.ClearSelected();
                }

                if (ImGui::Button("Run File", ImVec2(-1.0f, 0.0f)))
                {
                    t1 = std::thread(run_file, ricom, filename, width, height);
                    b_running = true;
                    t1.detach();
                }
            }
            ImGui::Separator();
            ImGui::BeginChild("Progress", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar);

            if (b_running)
            {
                ImGui::ProgressBar(ricom->fr_count / (width * height), ImVec2(-1.0f, 0.0f));
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
                                     GL_RGBA, GL_UNSIGNED_BYTE, ricom->srf_cbed->pixels);
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
                                 GL_RGBA, GL_UNSIGNED_BYTE, ricom->srf_ricom->pixels);
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
                                 GL_RGBA, GL_UNSIGNED_BYTE, ricom->srf_stem->pixels);
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

        // if (ricom->rc_quit || (nxy - ricom->fr_count < 1))
        // {
        //     if (t1.joinable())
        //     {
        //         t1.join();
        //     }
        //     if (t2.joinable())
        //     {
        //         t2.~thread();
        //     }
        // b_connected = false;
        // ricom->rc_quit = false;
        // }
    }

    // Cleanup
    // if (ricom->srf_ricom != NULL)
    // SDL_FreeSurface(ricom->srf_ricom);
    // if (ricom->srf_stem != NULL)
    // SDL_FreeSurface(ricom->srf_stem);

    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();

    // delete ricom;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
