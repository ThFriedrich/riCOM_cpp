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
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "imfilebrowser.h"

#include "libnpy.hpp"
#include "tinycolormap.hpp"
#include "fft2d.hpp"

enum class GIM_Flags : unsigned char
{
    None                = 0,
    SaveImButton        = 1<<1,
    SaveDataButton      = 1<<2,
    ColormapSelector    = 1<<3,
    FftButton           = 1<<4,
    PowerSlider         = 1<<5,
};

GIM_Flags operator&(GIM_Flags lhs, GIM_Flags rhs);
GIM_Flags operator|(GIM_Flags lhs, GIM_Flags rhs);

class Generic_Image_Window
{
public:
    Generic_Image_Window(std::string title, GLuint *tex_id, bool auto_render, int data_cmap, GIM_Flags flags = GIM_Flags::None);
    void set_data(int width, int height, std::vector<float> *data); 
    void render_window(bool b_redraw, int last_y, int render_update_offset, bool b_trigger_update);
    void render_window(bool b_redraw, int last_y, bool b_trigger_update);
    void link_visibility_bool(bool* visible);
    void reset_min_max();
    void set_nx_ny(int width, int height); 
    bool *pb_open;
    Generic_Image_Window *fft_window;
private:
    // Window properties
    std::string title;
    GIM_Flags flags;
    // ImVec2 pos;
    ImVec2 size;
    ImVec2 uv_min;
    ImVec2 uv_max;
    ImVec4 tint_col;
    ImVec4 border_col;
    bool b_open;
    bool b_trigger_update;

    // Interfaces
    ImGuiIO &io = ImGui::GetIO();
    ImGui::FileBrowser saveFileDialog;
    ImGui::FileBrowser saveDataDialog;

    // Image properties
    float start_x, start_y;         // Scrolling/Panning positions
    float start_xs, start_ys;       // Scrolling/Panning positions
    const char *cmaps[12] = {"Parula", "Heat", "Jet", "Turbo", "Hot", "Gray", "Magma", "Inferno", "Plasma", "Viridis", "Cividis", "Github"};
    int data_cmap;
    float zoom;
    float power;
    bool auto_render;

    // Data Properties
    int nx;
    int ny;
    int nxy;
    int last_y;
    int last_idr;
    int render_update_offset;       // Accounts for ricom Kernel size
    float data_min;
    float data_max;
    float data_range;
    std::vector<float> *data;
    bool b_data_set;

    // FFT data
    dcvector data_fft;
    std::vector<float> data_fft_f;
    dcvector data_val;

    // Surface and Texture
    SDL_Surface *sdl_srf;
    GLuint *tex_id;

    // Methods
    inline void render_image(int ye);
    inline void render_image();
    inline void set_min_max();
    inline void set_min_max(int last_y);
    inline void reset_limits();
    inline void set_pixel(int idx, int idy);
    inline void draw_pixel(int x, int y, float val);
    inline void compute_fft();
    inline bool has(GIM_Flags flag);
};

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