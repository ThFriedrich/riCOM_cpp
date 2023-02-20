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

#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include <string>
#include <vector>
#include <complex>
#include <filesystem>

#include <SDL_image.h>

#include "imgui.h"
#include "imgui_internal.h"

#include "libnpy.hpp"

class Main_Dock
{
    public:
        ImGuiID dock_id;
        const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        Main_Dock(ImGuiID dock_id);
        void render(ImVec2 pos, ImVec2 size);
};

template <typename T>
void save_numpy(std::string *path, int nx, int ny, std::vector<T> *data);

void save_image(std::string *path, SDL_Surface *sdl_srf);

void v_splitter(float thickness, float &size0, float &min_h, float &max_h, float offset);
#endif // GUI_UTILS_H