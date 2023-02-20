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

#include "ImGuiImageWindow.h"

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

template<typename T>
bool ImGuiImageWindow<T>::has(GIM_Flags flag)
{
    return static_cast<bool>(flags & flag);
}

// Redraws the entire image
template<typename T>
void ImGuiImageWindow<T>::render_image()
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
template<typename T>
void ImGuiImageWindow<T>::render_image(int last_idr)
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

template<>
void ImGuiImageWindow<float>::set_pixel(int idx, int idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = pw((data->at(idr) - data_min) / data_range, power);

    // Update pixel at location
    draw_pixel(idx, idy, val);
}

template<>
void ImGuiImageWindow<std::complex<float>>::set_pixel(int idx, int idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;

    // Get magnitude and angle from complex
    float mag = (abs(data->at(idr)) - data_min) / data_range;
    float ang = arg(data->at(idr));
    ang = (ang/M_PI+1)/2;
    mag = pw(mag,power);

    // Update pixel at location
    draw_pixel(idx, idy, ang, mag);
}

template<>
float ImGuiImageWindow<float>::get_val(int idr)
{
    return (*data)[idr];
}

template<>
float ImGuiImageWindow<std::complex<float>>::get_val(int idr)
{
    return abs((*data)[idr]);
}

template<typename T>
void ImGuiImageWindow<T>::set_min_max(int last_idr)
{
    float val;
    for (int idr = this->last_idr; idr < last_idr; idr++)
    {
        val = get_val(idr);
        if (val < data_min)
        {
            data_min = val;
            data_range = data_max - data_min;
            b_trigger_update = true;
        }
        if (val > data_max)
        {
            data_max = val;
            data_range = data_max - data_min;
            b_trigger_update = true;
        }
    }
}

template<typename T>
void ImGuiImageWindow<T>::set_min_max()
{
    float val;
    for (int idr = 0; idr < nxy; idr++)
    {
        val = get_val(idr);
        if (val < data_min)
        {
            data_min = val;
            data_range = data_max - data_min;
            b_trigger_update = true;
        }
        if (val > data_max)
        {
            data_max = val;
            data_range = data_max - data_min;
            b_trigger_update = true;
        }
    }
}

// Draw a pixel on the surface at (x, y) for a given colormap
template<typename T>
void ImGuiImageWindow<T>::draw_pixel(int x, int y, float val)
{
    cmap::Color c = cmap::GetColor(val, cmap::ColormapType(data_cmap));
    Uint32 px = SDL_MapRGB(sdl_srf->format, (Uint8)(c.ri()), (Uint8)(c.gi()), (Uint8)(c.bi()));
    Uint32 *const target_pixel = (Uint32 *)((Uint8 *)sdl_srf->pixels + y * sdl_srf->pitch + x * sdl_srf->format->BytesPerPixel);
    *target_pixel = px;
}

template<typename T>
void ImGuiImageWindow<T>::draw_pixel(int x, int y, float ang, float mag)
{
    
    cmap::Color c = mag*cmap::GetColor(ang, cmap::ColormapType(data_cmap));
    Uint32 px = SDL_MapRGB(sdl_srf->format, (Uint8)(c.ri()), (Uint8)(c.gi()), (Uint8)(c.bi()));
    Uint32 *const target_pixel = (Uint32 *)((Uint8 *)sdl_srf->pixels + y * sdl_srf->pitch + x * sdl_srf->format->BytesPerPixel);
    *target_pixel = px;
}

template<typename T>
ImGuiImageWindow<T>::ImGuiImageWindow(std::string title, GLuint *tex_id, bool auto_render, int data_cmap,  GIM_Flags flags, bool *visible)
{

    this->title = title;
    this->flags = flags;
    this->tex_id = tex_id;
    this->pb_open = visible;
    this->auto_render = auto_render;
    this->data_cmap = data_cmap;
    this->last_y = 0;
    this->last_idr = 0;
    this->last_img = 0;
    this->zoom = 1.0f;
    this->power = 1.0f;
    this->ny = 1;
    this->nx = 1;
    this->nxy = 1;
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

template<typename T>
void ImGuiImageWindow<T>::set_data(int width, int height, std::vector<T> *data)
{
    this->data = data;
    set_nx_ny(width, height);
    reset_limits();
    if (!auto_render)
    {
        render_image();
    };
    b_data_set = true;
}

template<typename T>
void ImGuiImageWindow<T>::reset_limits()
{
    last_y = 0;
    last_idr = 0;
    last_img = 0;
    data_min = FLT_MAX;
    data_max = -FLT_MAX;
    data_range = FLT_MAX;
}

template<typename T>
void ImGuiImageWindow<T>::set_nx_ny(int width, int height)
{
    this->nx = width;
    this->ny = height;
    this->nxy = height * width;
    
    data_fft.resize(nxy);
    data_fft_f.resize(nxy);
    data_val.resize(nxy);

    reset_limits();

    sdl_srf = SDL_CreateRGBSurface(0, this->nx, this->ny, 32, 0, 0, 0, 0);
    if (sdl_srf == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
}

template<typename T>
void ImGuiImageWindow<T>::render_window(bool b_redraw, int last_y, int render_update_offset, bool b_trigger_update)
{
    this->render_update_offset = render_update_offset;
    render_window(b_redraw, last_y, b_trigger_update);
}

template<typename T>
void ImGuiImageWindow<T>::reset_min_max()
{
    data_min = FLT_MAX;
    data_max = -FLT_MAX;
    data_range = FLT_MAX;
}

template<>
void ImGuiImageWindow<float>::compute_fft()
{
    FFT2D fft2d(ny, nx, ny, nx);
    FFT2D::r2c(*data, data_val);
    fft2d.fft(data_val, data_fft);
    FFT2D::c2r(data_fft, data_fft_f);
}

template<>
void ImGuiImageWindow<std::complex<float>>::compute_fft()
{
    FFT2D fft2d(ny, nx, ny, nx);
    fft2d.fft(*data, data_fft);
    FFT2D::c2r(data_fft, data_fft_f);
}

// Deal with situation when process is finished (redraw==false) but not fully rendered
// Should also render at end of a full cycle but not when it's finished completely
template<typename T>
bool ImGuiImageWindow<T>::detect_frame_switch(int &fr_count)
{
    int n_im = (fr_count) / nxy;
    if (last_img < n_im)
    {
        last_img = n_im;
        fr_count = nxy;
        this->last_y = 0;
        return true;
    } else {
        fr_count -= (n_im * nxy);
        return false;
    }
}

template<>
void ImGuiImageWindow<float>::value_tooltip(const int x, const int y, const float zoom)
{
    float val = 0.0f;
    if (b_data_set)
        val = data->at(y * nx + x);
    ImGui::BeginTooltip();
    ImGui::Text("XY: %i, %i", x, y);
    ImGui::Text("Value: %.2f", val);
    ImGui::Text("Zoom: %.2f", zoom);
    ImGui::EndTooltip();
}

template<>
void ImGuiImageWindow<std::complex<float>>::value_tooltip(const int x, const int y, const float zoom)
{
    std::complex<float> val = 0.0;
    if (b_data_set)
        val = data->at(y * nx + x);
    ImGui::BeginTooltip();
    ImGui::Text("XY: %i, %i", x, y);
    ImGui::Text("Angle: %.2f", arg(val));
    ImGui::Text("Magnitude: %.2f", abs(val));
    ImGui::Text("Zoom: %.2f", zoom);
    ImGui::EndTooltip();
}

// b_redraw is the standard timer based update, b_trigger_ext can trigger a full redraw of the image
template<typename T>
void ImGuiImageWindow<T>::render_window(bool b_redraw, int fr_count, bool b_trigger_ext)
{
    ImGui::SetNextWindowSize(ImVec2{256, 256}, ImGuiCond_FirstUseEver);
    bool t_open = ImGui::Begin(title.c_str(), pb_open, ImGuiWindowFlags_NoScrollbar);
    if (t_open)
    {
        bool fr_switch = detect_frame_switch(fr_count);
        b_trigger_update = b_trigger_update || b_trigger_ext || fr_switch;
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
                saveFileDialog.ClearSelected();
                save_image(&img_file, sdl_srf);
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
                saveDataDialog.ClearSelected();
                save_numpy(&com_file, nx, ny, data);
            }
        }

        if (this->has(GIM_Flags::FftButton))
        {
            ImGui::SameLine();
            bool fft_button_press = ImGui::Button("Compute FFT");
            if (fft_button_press)
            {
                *fft_window->pb_open = true;
            }
            if (fft_button_press || (*fft_window->pb_open && b_trigger_update))
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
                render_image((fr_count==0)?nxy:fr_count);
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
                render_image((fr_count==0)?nxy:fr_count);
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
            if ((std::abs(dz) > 0.0f) || ImGui::IsMouseClicked(ImGuiMouseButton_Left))
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
            if (std::abs(dz) > 0.0f)
            {

                float zoom2 = zoom + dz * 0.1;
                zoom2 = (std::max)(1.0f, zoom2);

                float dx = ((xy.x - pos.x)/tex_w_z) * tex_w * (zoom2-zoom);
                float dy = ((xy.y - pos.y)/tex_h_z) * tex_h * (zoom2-zoom);

                ImGui::SetScrollX(start_xs + dx);
                ImGui::SetScrollY(start_ys + dy);
               
                zoom = zoom2;
            }

            // Value Popup
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
                float scale_fct = scale * zoom;
                int x = (int)std::floor((xy.x - pos.x) / scale_fct);
                int y = (int)std::floor((xy.y - pos.y) / scale_fct);
                value_tooltip(x,y,zoom);
            }
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                zoom = 1.0f;
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }
        }
        ImGui::EndChildFrame();
    }
    ImGui::End();
    b_trigger_update = false;
}

template class ImGuiImageWindow<float>;
template class ImGuiImageWindow<std::complex<float>>;