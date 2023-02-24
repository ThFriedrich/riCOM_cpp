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

#ifndef RICOM_H
#define RICOM_H

#ifdef _WIN32
#include <io.h>
#pragma warning(disable : 4005 4333 34)
#else
#include <unistd.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdio.h>
#include <complex>
#include <cfloat>
#include <vector>
#include <string>
#include <SDL.h>
#include <mutex>
#include <future>
#include <thread>
#include <fftw3.h>
#include <chrono>
#include <algorithm>

#include "BoundedThreadPool.hpp"
#include "tinycolormap.hpp"
#include "fft2d.hpp"
#include "SocketConnector.h"
#include "ProgressMonitor.h"
#include "MerlinInterface.h"
#include "TimepixInterface.h"
#include "Camera.h"
#include "GuiUtils.h"

namespace chc = std::chrono;

class Ricom_kernel
{
public:
    // Properties
    int kernel_size;
    bool b_filter;
    std::array<int, 2> kernel_filter_frequency;
    int k_width_sym;
    int k_area;
    float rotation;
    std::vector<float> kernel_x;
    std::vector<float> kernel_y;
    std::vector<float> kernel_filter;
    std::vector<float> f_approx;
    SDL_Surface *srf_kx;
    SDL_Surface *srf_ky;
    // Methods
    void compute_kernel();
    void compute_filter();
    void include_filter();
    std::vector<int> fftshift_map(int x, int y);
    // Constructor
    Ricom_kernel() : kernel_size(5),
                     b_filter(false),
                     kernel_filter_frequency{1, 4},
                     k_width_sym(0),
                     k_area(0),
                     rotation(0.0),
                     kernel_x(),
                     kernel_y(),
                     kernel_filter(),
                     f_approx(),
                     srf_kx(), srf_ky()
    {
        compute_kernel();
    };
    void approximate_frequencies(size_t n_im);
    void draw_surfaces();
    // Destructor
    ~Ricom_kernel(){};
};

////////////////////////////////////////////////
//    Helper class for ricom data indexing    //
////////////////////////////////////////////////
class id_x_y
{
public:
    int id;
    bool valid;
    id_x_y() : id(0), valid(false){};
    id_x_y(int id, bool valid);
};

class Update_list
{
private:
    // Properties
    Ricom_kernel kernel;
    int nx;
    int ny;

public:
    // Properties
    std::vector<id_x_y> ids;
    // Methods
    void init(Ricom_kernel kernel, int nx_ricom, int ny_ricom);
    inline void shift(id_x_y &id_sft, int id, int shift);
    // Constructor
    Update_list() : kernel(), nx(0), ny(0){};
};

class Ricom_detector
{
public:
    // Properties
    std::array<float, 2> radius;
    std::array<float, 2> radius2;
    std::vector<int> id_list;

    // Methods
    void compute_detector(int nx_cam, int ny_cam, std::array<float, 2> &offset);
    // Constructor
    Ricom_detector() : radius{0, 0}, radius2{0, 0}, id_list(){};
    // Destructor
    ~Ricom_detector(){};
};

namespace RICOM
{
    enum modes
    {
        FILE,
        TCP
    };
    void run_ricom(Ricom *r, RICOM::modes mode);
    void run_connection_script(Ricom *r, MerlinSettings *merlin, const std::string &python_path);
}

class Ricom
{
private:
    // vSTEM Variables
    float stem_max;
    float stem_min;

    // ricom variables
    std::vector<int> u;
    std::vector<int> v;

    Update_list update_list;

    // Electric field magnitude
    float e_mag_max;
    float e_mag_min;

    // Variables for potting in the SDL2 frame
    float ricom_max;
    float ricom_min;
    std::vector<float> cbed_log;

    // Thread Synchronization Variables
    std::mutex ricom_mutex;
    std::mutex stem_mutex;
    std::mutex counter_mutex;
    std::mutex e_field_mutex;

    // Private Methods - General
    void init_surface();
    template <typename T>
    inline void update_surfaces(int iy, std::vector<T> *p_frame);
    void reinit_vectors_limits();
    void reset_limits();
    void reset_file();
    void calculate_update_list();
    inline void rescales_recomputes();
    template <typename T, class CameraInterface>
    inline void skip_frames(int n_skip, std::vector<T> &data, CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> *camera_fr);
    template <typename T>
    inline void swap_endianess(T &val);

    // Private Methods - riCOM
    inline void icom(std::array<float, 2> *com, int x, int y);
    inline void icom(std::array<float, 2> com, int x, int y);
    template <typename T>
    inline void com(std::vector<T> *data, std::array<float, 2> &com);
    template <typename T>
    void read_com_merlin(std::vector<T> &data, std::array<float, 2> &com);
    inline void set_ricom_pixel(int idx, int idy);
    template <typename T>
    inline void com_icom(std::vector<T> data, int ix, int iy, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon);
    template <typename T>
    inline void com_icom(std::vector<T> *p_data, int ix, int iy, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon);

    // Private Methods - vSTEM
    template <typename T>
    inline void stem(std::vector<T> *data, size_t id_stem);
    inline void set_stem_pixel(size_t idx, size_t idy);

    // Private Methods electric field
    inline void compute_electric_field(std::array<float, 2> &p_com_xy, size_t id);
    inline void set_e_field_pixel(size_t idx, size_t idy);

public:
    SocketConnector socket;
    std::string file_path;
    CAMERA::Camera_BASE camera;
    RICOM::modes mode;
    bool b_print2file;
    int redraw_interval;
    int last_y;
    ProgressMonitor *p_prog_mon;
    bool b_busy;
    bool update_offset;
    bool b_vSTEM;
    bool b_e_mag;
    bool b_plot_cbed;
    bool b_plot2SDL;
    bool b_recompute_detector;
    bool b_recompute_kernel;
    Ricom_detector detector;
    Ricom_kernel kernel;
    std::array<float, 2> offset;
    std::array<float, 2> com_public;
    std::vector<float> com_map_x;
    std::vector<float> com_map_y;
    std::vector<float> ricom_data;
    std::vector<float> stem_data;
    std::vector<std::complex<float>> e_field_data;

    // Scan Area Variables
    int nx;
    int ny;
    int nxy;
    int rep;
    int fr_total;
    int skip_row;
    int skip_img;

    // Variables for progress and performance
    int n_threads;
    int n_threads_max;
    int queue_size;
    float fr_freq;        // Frequncy per frame
    float fr_count;       // Count all Frames processed in an image
    float fr_count_total; // Count all Frames in a scanning session
    std::atomic<bool> rescale_ricom;
    std::atomic<bool> rescale_stem;
    std::atomic<bool> rescale_e_mag;
    bool rc_quit;

    SDL_Surface *srf_ricom; // Surface for the ricom window;
    int ricom_cmap;
    SDL_Surface *srf_stem; // Surface for the vSTEM window;
    int stem_cmap;
    SDL_Surface *srf_cbed; // Surface for the CBED window;
    int cbed_cmap;
    SDL_Surface *srf_e_mag; // Surface for the E-Field window;
    int e_mag_cmap;

    // Public Methods
    void draw_ricom_image();
    void draw_ricom_image(int y0, int ye);
    void draw_stem_image();
    void draw_stem_image(int y0, int ye);
    void draw_e_field_image();
    void draw_e_field_image(int y0, int ye);
    template <class CameraInterface>
    void run_reconstruction(RICOM::modes mode);
    void reset();
    template <typename T>
    void plot_cbed(std::vector<T> *p_data);
    template <typename T, class CameraInterface>
    void process_data(CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> *camera);
    template <class CameraInterface>
    void process_data(CAMERA::Camera<CameraInterface, CAMERA::EVENT_BASED> *camera);
    enum CAMERA::Camera_model select_mode_by_file(const char *filename);

    // Constructor
    Ricom();

    // Destructor
    ~Ricom();
};

#endif // __RICOM_H__