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

#include "GuiUtils.h"

template <typename T>
void save_numpy(std::string *path, int nx, int ny, std::vector<T> *data)
{
    std::string ext = std::filesystem::path(*path).extension().string();
    if (ext != ".npy")
    {
        *path += ".npy";
    }
    const std::vector<long unsigned> shape{static_cast<long unsigned>(ny), static_cast<long unsigned>(nx)};
    npy::SaveArrayAsNumpy(path->c_str(), false, shape.size(), shape.data(), *data);
}
template void save_numpy<float>(std::string *path, int nx, int ny, std::vector<float> *data);
template void save_numpy<std::complex<float>>(std::string *path, int nx, int ny, std::vector<std::complex<float>> *data);

void save_image(std::string *path, SDL_Surface *sdl_srf)
{
    std::string ext = std::filesystem::path(*path).extension().string();
    if ((ext != ".png") && (ext != ".PNG"))
    {
        *path += ".png";
    }
    IMG_SavePNG(sdl_srf, path->c_str());
}

// Vertical Splitter Container
void v_splitter(float thickness, float &size0, const float &min_h, const float &max_h, const float &offset)
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

namespace SDL_Utils
{
    // Draw a pixel on the surface at (x, y) for a given colormap
    void draw_pixel(SDL_Surface *surface, int x, int y, float val, int col_map)
    {
        tinycolormap::Color c = tinycolormap::GetColor(val, tinycolormap::ColormapType(col_map));
        Uint32 px = SDL_MapRGB(surface->format, (Uint8)(c.ri()), (Uint8)(c.gi()), (Uint8)(c.bi()));
        Uint32 *const target_pixel = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
        *target_pixel = px;
    }

    void draw_pixel(SDL_Surface *surface, int x, int y, float ang, float mag, int col_map)
    {
        tinycolormap::Color c = mag * tinycolormap::GetColor(ang, tinycolormap::ColormapType(col_map));
        Uint32 px = SDL_MapRGB(surface->format, (Uint8)(c.ri()), (Uint8)(c.gi()), (Uint8)(c.bi()));
        Uint32 *const target_pixel = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
        *target_pixel = px;
    }
}