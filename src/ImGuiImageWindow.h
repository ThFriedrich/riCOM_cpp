#include <vector>
#include <string>
#include <stdio.h>
#include <algorithm>
#include <SDL.h>
#include <SDL_opengl.h>

#include "tinycolormap.hpp"
#include "fft2d.hpp"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "imfilebrowser.h"

#include "GuiUtils.h"

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

template<class T>
class ImGuiImageWindow
{
public:
    ImGuiImageWindow(std::string title, GLuint *tex_id, bool auto_render, int data_cmap, GIM_Flags flags = GIM_Flags::None, bool *visible=nullptr);
    void set_data(int width, int height, std::vector<T> *data); 
    void render_window(bool b_redraw, int last_y, int render_update_offset, bool b_trigger_update);
    void render_window(bool b_redraw, int last_y, bool b_trigger_update);
    void reset_min_max();
    void set_nx_ny(int width, int height); 
    bool *pb_open;
    ImGuiImageWindow<float> *fft_window;
    bool b_trigger_update;
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

    // Interfaces
    ImGuiIO &io = ImGui::GetIO();
    ImGui::FileBrowser saveFileDialog;
    ImGui::FileBrowser saveDataDialog;

    // Image properties
    float start_x, start_y;         // Scrolling/Panning positions
    float start_xs, start_ys;       // Scrolling/Panning positions
    const char *cmaps[13] = {"Parula", "Heat", "Jet", "Turbo", "Hot", "Gray", "Magma", "Inferno", "Plasma", "Viridis", "Cividis", "Github","HSV"};
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
    int last_img;
    int render_update_offset;       // Accounts for ricom Kernel size
    float data_min;
    float data_max;
    float data_range;
    std::vector<T> *data;
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
    inline void draw_pixel(int x, int y, float ang, float mag);
    inline void compute_fft();
    inline bool has(GIM_Flags flag);
    inline bool detect_frame_switch(int &fr_count);
    inline float get_val(int idx);
    inline void value_tooltip(const int x, const int y, const float zoom);
};