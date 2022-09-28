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

#include "Gui_utils.h"

template <typename T>
inline T pw(T val, T power)
{
    return copysign(1.0, val) * pow(abs(val), power);
}

namespace cmap = tinycolormap;

GIM_Flags operator|(GIM_Flags lhs, GIM_Flags rhs)
{
    return static_cast<GIM_Flags>(
        static_cast<std::underlying_type_t<GIM_Flags>>(lhs) |
        static_cast<std::underlying_type_t<GIM_Flags>>(rhs));
}

GIM_Flags operator&(GIM_Flags lhs, GIM_Flags rhs)
{
    return static_cast<GIM_Flags>(
        static_cast<std::underlying_type_t<GIM_Flags>>(lhs) &
        static_cast<std::underlying_type_t<GIM_Flags>>(rhs));
}

bool Generic_Image_Window::has(GIM_Flags flag)
{
    return static_cast<bool>(flags & flag);
}

// Redraws the entire image
void Generic_Image_Window::render_image()
{
    if (b_data_set)
    {
        if (!auto_render)
        {
            set_min_max();
        }
        for (int y = 0; y < ny; y++)
        {
            for (int x = 0; x < nx; x++)
            {
                set_pixel(x, y);
            }
        }
    }
}

// Redraws the entire ricom image from line y0 to line ye
void Generic_Image_Window::render_image(int last_idr)
{
    int last_y = (last_idr / nx);
    if (b_data_set)
    {
        set_min_max(last_idr);
        for (int y = (std::max)(0, this->last_y - render_update_offset); y < (std::min)(last_y + render_update_offset, ny); y++)
        {
            for (int x = 0; x < nx; x++)
            {
                set_pixel(x, y);
            }
        }
    }
    this->last_y = last_y;
    this->last_idr = last_idr;
}

void Generic_Image_Window::set_pixel(int idx, int idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = pw((data->at(idr) - data_min) / data_range, power);

    // Update pixel at location
    draw_pixel(idx, idy, val);
}

void Generic_Image_Window::set_min_max(int last_idr)
{
    float val;
    for (int idr = this->last_idr; idr < last_idr; idr++)
    {
        val = (*data)[idr];
        if (val < data_min)
        {
            data_min = val;
            data_range = data_max - data_min;
        }
        if (val > data_max)
        {
            data_max = val;
            data_range = data_max - data_min;
        }
    }
}

void Generic_Image_Window::set_min_max()
{
    data_min = *std::min_element(data->begin(), data->end());
    data_max = *std::max_element(data->begin(), data->end());
    data_range = data_max - data_min;
}

// Draw a pixel on the surface at (x, y) for a given colormap
void Generic_Image_Window::draw_pixel(int x, int y, float val)
{
    cmap::Color c = cmap::GetColor(val, cmap::ColormapType(data_cmap));
    Uint32 px = SDL_MapRGB(sdl_srf->format, (Uint8)(c.r() * 255), (Uint8)(c.g() * 255), (Uint8)(c.b() * 255));
    Uint32 *const target_pixel = (Uint32 *)((Uint8 *)sdl_srf->pixels + y * sdl_srf->pitch + x * sdl_srf->format->BytesPerPixel);
    *target_pixel = px;
}

Generic_Image_Window::Generic_Image_Window(std::string title, GLuint *tex_id, bool auto_render, int data_cmap, GIM_Flags flags)
{

    this->title = title;
    this->flags = flags;
    this->tex_id = tex_id;
    this->b_open = false;
    this->pb_open = &this->b_open;
    this->auto_render = auto_render;
    this->data_cmap = data_cmap;
    this->last_y = 0;
    this->zoom = 1.0f;
    this->power = 1.0f;
    this->ny = 1;
    this->nx = 1;
    this->render_update_offset = 0;
    this->b_trigger_update = false;
    saveFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    saveFileDialog.SetTitle("Save " + title + " image as .png");
    saveDataDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    saveDataDialog.SetTitle("Save " + title + "-data as numpy array (.npy)");

    uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
    uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
    tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
    border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
    sdl_srf = SDL_CreateRGBSurface(0, this->nx, this->ny, 32, 0, 0, 0, 0);
    if (sdl_srf == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    this->b_data_set = false;
}

void Generic_Image_Window::link_visibility_bool(bool *visible)
{
    this->pb_open = visible;
}

void Generic_Image_Window::set_data(int width, int height, std::vector<float> *data)
{
    this->data = data;
    set_nx_ny(width, height);

    b_open = true;
    if (this->pb_open != NULL)
        this->pb_open = &b_open;
    reset_limits();
    if (!auto_render)
    {
        render_image();
    };
    b_data_set = true;
}

void Generic_Image_Window::reset_limits()
{
    last_y = 0;
    data_min = FLT_MAX;
    data_max = -FLT_MAX;
    data_range = FLT_MAX;
}

void Generic_Image_Window::set_nx_ny(int width, int height)
{
    this->nx = width;
    this->ny = height;
    this->nxy = height * width;
    reset_limits();

    sdl_srf = SDL_CreateRGBSurface(0, this->nx, this->ny, 32, 0, 0, 0, 0);
    if (sdl_srf == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
}

void Generic_Image_Window::render_window(bool b_redraw, int last_y, int render_update_offset, bool b_trigger_update)
{
    this->render_update_offset = render_update_offset;
    render_window(b_redraw, last_y, b_trigger_update);
}

void Generic_Image_Window::reset_min_max()
{
    data_min = FLT_MAX;
    data_max = -FLT_MAX;
    data_range = FLT_MAX;
}

void Generic_Image_Window::compute_fft()
{
    data_fft.resize(nxy);
    data_fft_f.resize(nxy);
    data_val.resize(nxy);

    FFT2D::r2c(*data, data_val);
    FFT2D fft2d(nx, ny, nx, ny);
    fft2d.fft(data_val, data_fft);
    FFT2D::c2r(data_fft, data_fft_f);
}

void Generic_Image_Window::render_window(bool b_redraw, int fr_count, bool b_trigger_ext)
{
    ImGui::SetNextWindowSize(ImVec2{256, 256}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(title.c_str(), pb_open, ImGuiWindowFlags_NoScrollbar))
    {
        // int frames_passed = ((fr_count-1)/nxy)*nxy;
        fr_count -= (((fr_count-1)/nxy)*nxy);
        b_trigger_update = b_trigger_update || b_trigger_ext;
        if (b_trigger_update)
        {
            render_image();
        }

        if (this->has(GIM_Flags::SaveImButton))
        {
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
                IMG_SavePNG(sdl_srf, img_file.c_str());
            }
        }

        if (this->has(GIM_Flags::SaveDataButton))
        {
            ImGui::SameLine();
            if (ImGui::Button("Save Data as..."))
            {
                saveDataDialog.Open();
            }
            saveDataDialog.Display();
            if (saveDataDialog.HasSelected())
            {
                std::string com_file = saveDataDialog.GetSelected().string();
                if (std::filesystem::path(com_file).extension() == "")
                {
                    com_file += ".npy";
                }
                saveDataDialog.ClearSelected();
                const std::vector<long unsigned> shape{static_cast<long unsigned>(ny), static_cast<long unsigned>(nx)};
                npy::SaveArrayAsNumpy(com_file, false, shape.size(), shape.data(), *data);
            }
        }

        if (this->has(GIM_Flags::FftButton))
        {
            ImGui::SameLine();
            if (ImGui::Button("Compute FFT") || (*fft_window->pb_open && b_trigger_update))
            {
                if (b_data_set)
                {
                    compute_fft();
                    fft_window->set_data(nx, ny, &data_fft_f);
                    fft_window->b_trigger_update = true;
                }
                else
                {
                    std::cout << "FFT was not performed, because no data was found in " + this->title + "!" << std::endl;
                }
            }
        }

        if (this->has(GIM_Flags::PowerSlider))
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(64);
            if (ImGui::DragFloat("Power", &power, 0.05f, 0.05f, 2.0f, "%.2f"))
            {
                this->last_idr = 0;
                this->last_y = 0;
                render_image(fr_count);
                b_trigger_update = true;
            }
        }

        if (this->has(GIM_Flags::ColormapSelector))
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::Combo("Colormap", &data_cmap, cmaps, IM_ARRAYSIZE(cmaps)))
            {
                this->last_idr = 0;
                this->last_y = 0;
                render_image(fr_count);
                b_trigger_update = true;
            }
        }

        if (b_redraw && auto_render && fr_count > 0)
            render_image(fr_count);

        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::BeginChildFrame(ImGui::GetID("ImageFrame"), ImVec2(0.0f, 0.0f), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImVec2 vAvail = ImGui::GetContentRegionAvail();
        float scale = (std::min)(vAvail.x / sdl_srf->w, vAvail.y / sdl_srf->h);
        float tex_h = sdl_srf->h * scale;
        float tex_w = sdl_srf->w * scale;
        float tex_h_z = tex_h * zoom;
        float tex_w_z = tex_w * zoom;
        if (b_redraw || b_trigger_update)
        {
            glBindTexture(GL_TEXTURE_2D, (*tex_id));
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sdl_srf->w, sdl_srf->h, 0,
                         GL_BGRA, GL_UNSIGNED_BYTE, sdl_srf->pixels);
        }
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::Image((ImTextureID)(*tex_id), ImVec2(tex_w_z, tex_h_z), uv_min, uv_max, tint_col, border_col);
        if (ImGui::IsItemHovered())
        {

            // Get Mouse Inputs
            float dz = (float)io.MouseWheel;
            ImVec2 xy = ImGui::GetMousePos();

            // Compute relative cursor positions
            float rel_x = xy.x - pos.x - ImGui::GetScrollX();
            float rel_y = xy.y - pos.y - ImGui::GetScrollY();

            // Adjust Scroll positions
            // Capture Start position of scroll or drag/pan
            if (dz > 0.0f || ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                start_x = rel_x;
                start_y = rel_y;
                start_xs = ImGui::GetScrollX();
                start_ys = ImGui::GetScrollY();
            }

            // Panning
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                ImGui::SetScrollX(start_xs - (rel_x - start_x));
                ImGui::SetScrollY(start_ys - (rel_y - start_y));
            }

            // Zooming
            else if (std::abs(dz) > 0.0f)
            {
                zoom += dz * 0.1;
                zoom = (std::max)(1.0f, zoom);
                ImGui::SetScrollX(start_x * (zoom - 1.0f));
                ImGui::SetScrollY(start_y * (zoom - 1.0f));
            }

            // Value Popup
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
                float scale_fct = scale * zoom;
                int x = (int)std::floor((xy.x - pos.x) / scale_fct);
                int y = (int)std::floor((xy.y - pos.y) / scale_fct);
                float val = (*data)[y * nx + x];
                ImGui::BeginTooltip();
                ImGui::Text("XY: %i, %i", x, y);
                ImGui::Text("Value: %.2f", val);
                ImGui::Text("Zoom: %.1f", zoom);
                ImGui::EndTooltip();
            }
        }
        ImGui::EndChildFrame();
    }
    ImGui::End();
    b_trigger_update = false;
}

// Vertical Splitter Container
void v_splitter(float thickness, float &size0, float &min_h, float &max_h, float offset)
{

    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab]);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabHovered]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive]);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().ScrollbarRounding);
    ImGui::Button("v", ImVec2(-1, thickness));
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
};

Main_Dock::Main_Dock(ImGuiID dock_id)
{
    this->dock_id = dock_id;
}

void Main_Dock::render(ImVec2 pos, ImVec2 size)
{
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("DockWindow", nullptr, window_flags);
    ImGui::DockSpace(dock_id);

    static auto first_time = true;
    if (first_time)
    {
        first_time = false;
        ImGui::DockBuilderRemoveNode(dock_id); // clear any previous layout
        ImGui::DockBuilderAddNode(dock_id, dockspace_flags);
        ImGui::DockBuilderSetNodePos(dock_id, pos);
        ImGui::DockBuilderSetNodeSize(dock_id, size);
        ImGui::DockBuilderDockWindow("RICOM", dock_id);
        ImGui::DockBuilderFinish(dock_id);
    }
    ImGui::End();
}
