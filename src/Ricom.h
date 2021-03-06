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

#include <stdio.h>
#include <cmath>
#include <complex>
#include <cfloat>
#include <vector>
#include <string>
#include <SDL.h>
#include <mutex>
#include <future>

#include "SocketConnector.h"
#include "ProgressMonitor.h"
#include "Camera.h"

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
    // Methods
    void compute_kernel();
    void compute_filter();
    void include_filter();
    std::vector<int> fftshift_map(int x, int y);
    // Constructor
    Ricom_kernel() : kernel_size(5),
                     b_filter(false),
                     kernel_filter_frequency{20, 1},
                     k_width_sym(0),
                     rotation(0.0),
                     kernel_x(),
                     kernel_y()
    {
        compute_kernel();
    };
    void approximate_frequencies(size_t n_im);
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
    int x;
    int y;
    int nx_ricom;
    int ny_ricom;
    bool valid;
    id_x_y() : id(0), x(0), y(0), nx_ricom(0), ny_ricom(0), valid(true){};
    id_x_y(int id, int x, int y, int nx_ricom, int ny_ricom, bool valid);
    friend id_x_y operator+(id_x_y &c1, const int &c2);
};

class Ricom_detector
{
public:
    // Properties
    std::array<float, 2> radius;
    std::vector<int> id_list;

    // Methods
    void compute_detector(int nx_cam, int ny_cam);
    void compute_detector(std::array<float, 2> &offset, int nx_cam, int ny_cam);
    // Constructor
    Ricom_detector(int nx_cam, int ny_cam) : radius{0.0, 0.0},
                                             id_list()
    {
        compute_detector(nx_cam, ny_cam);
    };
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
}

class Ricom
{
private:
    // vSTEM Variables
    std::vector<float> stem_data;
    float stem_max;
    float stem_min;

    // ricom variables
    std::vector<int> u;
    std::vector<int> v;
    std::vector<float> ricom_data;
    std::vector<id_x_y> update_list;

    // Variables for potting in the SDL2 frame
    float ricom_max;
    float ricom_min;

    // Thread Synchronization Variables
    std::mutex ricom_mutex;
    std::mutex stem_mutex;
    std::mutex counter_mutex;
    int last_y;

    // Private Methods - General
    void init_surface();
    void draw_pixel(SDL_Surface *surface, int x, int y, float val, int color_map);
    void reinit_vectors_limits();
    void reset_limits();
    void reset_file();
    void calculate_update_list();
    inline void rescales_recomputes();
    template <typename T, class CameraInterface>
    void skip_frames(int n_skip, std::vector<T> &data, CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> *camera_fr);
    template <typename T>
    void swap_endianess(T &val);

    // Private Methods - riCOM
    void icom(std::array<float, 2> &com, int x, int y);
    template <typename T>
    void com(std::vector<T> *data, std::array<float, 2> &com, std::atomic<int> *dose_sum);
    template <typename T>
    void read_com_merlin(std::vector<T> &data, std::array<float, 2> &com, int &dose_sum);
    void set_ricom_pixel(int idx, int idy);
    template <typename T>
    void com_icom(std::vector<T> data, int ix, int iy, std::atomic<int> *dose_sum, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon);
    template <typename T>
    void com_icom(std::vector<T> *p_data, int ix, int iy, std::atomic<int> *dose_sum, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon);

    // Private Methods - vSTEM
    template <typename T>
    void stem(std::vector<T> *data, size_t id_stem);
    void set_stem_pixel(size_t idx, size_t idy);

public:
    SocketConnector socket;
    std::string file_path;
    CAMERA::Camera_BASE camera;
    RICOM::modes mode;
    bool b_print2file;
    int redraw_interval;
    ProgressMonitor *p_prog_mon;
    bool b_busy;
    float update_dose_lowbound;
    bool update_offset;
    bool b_vSTEM;
    bool b_recompute_detector;
    bool b_recompute_kernel;
    Ricom_detector detector;
    Ricom_kernel kernel;
    std::array<float, 2> offset;
    std::array<float, 2> com_public;
    std::vector<float> com_map_x;
    std::vector<float> com_map_y;

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
    bool rc_quit;

    SDL_Surface *srf_ricom; // Surface for the window;
    int ricom_cmap;
    SDL_Surface *srf_stem; // Surface for the window;
    int stem_cmap;
    SDL_Surface *srf_cbed; // Surface for the window;
    int cbed_cmap;

    // Public Methods
    void draw_ricom_image();
    void draw_stem_image();
    void draw_ricom_image(int y0, int ye);
    void draw_stem_image(int y0, int ye);
    template <class CameraInterface>
    void run_reconstruction(RICOM::modes mode);
    void reset();
    template <typename T>
    void plot_cbed(std::vector<T> *p_data);
    template <typename T, class CameraInterface>
    void process_data(CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> *camera);
    template <class CameraInterface>
    void process_data(CAMERA::Camera<CameraInterface, CAMERA::EVENT_BASED> *camera);

    // Constructor
    Ricom();

    // Destructor
    ~Ricom();
};

#endif // __RICOM_H__