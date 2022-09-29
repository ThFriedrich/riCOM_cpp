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

#ifdef _WIN32
#include <io.h>
#pragma warning(disable : 4333 34)
#else
#include <unistd.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <cfloat>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <thread>
#include <future>
#include <fftw3.h>
#include "tinycolormap.hpp"

#include "Ricom.h"
#include "BoundedThreadPool.hpp"
#include "ProgressMonitor.h"
#include "MerlinInterface.h"
#include "TimepixInterface.h"

namespace chc = std::chrono;
namespace cmap = tinycolormap;

////////////////////////////////////////////////
// RICOM Kernel class method implementations  //
////////////////////////////////////////////////

// Compute the kernel
void Ricom_kernel::compute_kernel()
{
    float rot_rad = M_PI * rotation / 180;
    float cos_rot = cos(rot_rad);
    float sin_rot = sin(rot_rad);

    k_width_sym = kernel_size * 2 + 1;
    k_area = k_width_sym * k_width_sym;
    kernel_x.assign(k_area, 0);
    kernel_y.assign(k_area, 0);
    float d;
    float ix_sd;
    float iy_sd;
    int ix_s;
    int iy_s;
    int ix_e;
    int iy_e;

    for (int iy = 0; iy < k_width_sym; iy++)
    {
        iy_e = (iy + 1) * k_width_sym - 1;
        for (int ix = 0; ix < k_width_sym; ix++)
        {
            ix_s = ix - kernel_size;
            iy_s = iy - kernel_size;
            d = ix_s * ix_s + iy_s * iy_s;

            ix_e = k_area - iy_e + ix - 1;

            if (d > 0)
            {
                ix_sd = (ix_s / d);
                iy_sd = (iy_s / d);
                kernel_x[ix_e] = cos_rot * ix_sd - sin_rot * iy_sd;
                kernel_y[ix_e] = sin_rot * ix_sd + cos_rot * iy_sd;
            }
            else
            {
                kernel_y[ix_e] = 0;
                kernel_x[ix_e] = 0;
            }
        }
    }

    // Add filter
    if (b_filter)
    {
        compute_filter();
        include_filter();
    }
}

// Compute the filter
void Ricom_kernel::compute_filter()
{
    int filter_size = kernel_size * 2 + 1;
    int dist = 0;
    int ic = 0;
    kernel_filter.assign(filter_size * filter_size, 0);

    for (int iy = 0; iy < filter_size; iy++)
    {
        for (int ix = 0; ix < filter_size; ix++)
        {
            dist = pow(ix - kernel_size, 2) + pow(iy - kernel_size, 2);
            ic = iy * filter_size + ix;
            if (dist <= pow(kernel_filter_frequency[0], 2) && dist > pow(kernel_filter_frequency[1], 2))
            {
                kernel_filter[ic] = 1;
            }
        }
    }
}

// Applies the filter to the kernel
void Ricom_kernel::include_filter()
{
    int k_width_sym = kernel_size * 2 + 1;
    std::vector<int> map = fftshift_map(k_width_sym, k_width_sym);
    std::complex<float> *k_x = new std::complex<float>[k_area];
    std::complex<float> *k_y = new std::complex<float>[k_area];
    std::complex<float> *k_x_f = new std::complex<float>[k_area];
    std::complex<float> *k_y_f = new std::complex<float>[k_area];

    fftwf_plan px, py, ipx, ipy;

    for (int id = 0; id < k_area; id++)
    {
        k_x[id] = {kernel_x[id + map[id]], 0};
        k_y[id] = {kernel_y[id + map[id]], 0};
    }

    px = fftwf_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftwf_complex *>(k_x),
                          reinterpret_cast<fftwf_complex *>(k_x_f), FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(px);
    fftwf_destroy_plan(px);

    py = fftwf_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftwf_complex *>(k_y),
                          reinterpret_cast<fftwf_complex *>(k_y_f), FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(py);
    fftwf_destroy_plan(py);

    for (int id = 0; id < k_area; id++)
    {
        if (kernel_filter[id + map[id]] == 0)
        {
            k_x_f[id] = {0, 0};
            k_y_f[id] = {0, 0};
        }
    }

    ipx = fftwf_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftwf_complex *>(k_x_f),
                           reinterpret_cast<fftwf_complex *>(k_x), FFTW_BACKWARD, FFTW_ESTIMATE);
    fftwf_execute(ipx);
    fftwf_destroy_plan(ipx);

    ipy = fftwf_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftwf_complex *>(k_y_f),
                           reinterpret_cast<fftwf_complex *>(k_y), FFTW_BACKWARD, FFTW_ESTIMATE);
    fftwf_execute(ipy);
    fftwf_destroy_plan(ipy);

    for (int id = 0; id < k_area; id++)
    {
        kernel_x[id + map[id]] = k_x[id].real();
        kernel_y[id + map[id]] = k_y[id].real();
    }
    delete[] k_x;
    delete[] k_y;
    delete[] k_x_f;
    delete[] k_y_f;
}

// FFTSHIFT implementation
std::vector<int> Ricom_kernel::fftshift_map(int x, int y)
{
    int center_x = round(x / 2);
    int center_y = round(y / 2);
    std::vector<int> map(x * y);

    int shift_x, shift_y;
    if (x % 2 == 0)
    {
        shift_x = 0;
    }
    else
    {
        shift_x = -1;
    }
    if (y % 2 == 0)
    {
        shift_y = 0;
    }
    else
    {
        shift_y = -1;
    }

    int q1 = center_y * x + center_x;
    int q2 = center_y * x - (center_x + shift_x);
    int q3 = -1 * (center_y + shift_y) * x + center_x;
    int q4 = -1 * (center_y + shift_y) * x - (center_x + shift_x);

    int cnt = 0;
    for (int iy = 0; iy < center_y; iy++)
    {
        for (int ix = 0; ix < center_x; ix++)
        {
            map[cnt] = q1;
            cnt++;
        }
        for (int ix = center_x; ix < x; ix++)
        {
            map[cnt] = q2;
            cnt++;
        }
    }

    for (int iy = center_y; iy < y; iy++)
    {
        for (int ix = 0; ix < center_x; ix++)
        {
            map[cnt] = q3;
            cnt++;
        }
        for (int ix = center_x; ix < x; ix++)
        {
            map[cnt] = q4;
            cnt++;
        }
    }
    return map;
}

void Ricom_kernel::approximate_frequencies(size_t nx_im)
{
    f_approx.resize(nx_im);
    float f_max = 0;
    float k = kernel_size * 2;
    for (size_t i = 0; i < nx_im; i++)
    {
        float x = 2 * i * M_PI;
        f_approx[i] = (nx_im / x) * (1 - cos(k / 2 * (x / nx_im)));
        if (f_approx[i] > f_max)
        {
            f_max = f_approx[i];
        }
    }
    for (float &f : f_approx)
    {
        f /= f_max;
    }
}

///////////////////////////////////////////////////
//  RICOM detector class method implementations  //
///////////////////////////////////////////////////

// Computer detector distance map relative to the camera centre for vSTEM
void Ricom_detector::compute_detector(int nx_cam, int ny_cam)
{
    float rad_in2 = pow(radius[0], 2), rad_out2 = pow(radius[1], 2);
    float d2;
    id_list.clear();
    id_list.reserve(nx_cam * ny_cam);
    std::array<float, 2> offset = {((float)nx_cam - 1) / 2, ((float)ny_cam - 1) / 2};

    for (float iy = 0; iy < ny_cam; iy++)
    {
        for (float ix = 0; ix < nx_cam; ix++)
        {
            d2 = pow(ix, 2) + pow(iy - offset[1], 2);
            if (d2 > rad_in2 && d2 <= rad_out2)
            {
                id_list.push_back(iy * nx_cam + ix);
            }
        }
    }
}

// Computer detector distance map relative to a given centre (offset) for vSTEM
void Ricom_detector::compute_detector(std::array<float, 2> &offset, int nx_cam, int ny_cam)
{
    float rad_in2 = pow(radius[0], 2), rad_out2 = pow(radius[1], 2);
    float d2;
    id_list.clear();
    id_list.reserve(nx_cam * ny_cam);

    for (int iy = 0; iy < ny_cam; iy++)
    {
        for (int ix = 0; ix < nx_cam; ix++)
        {
            d2 = pow((float)ix - offset[0], 2) + pow((float)iy - offset[1], 2);
            if (d2 > rad_in2 && d2 <= rad_out2)
            {
                id_list.push_back(iy * nx_cam + ix);
            }
        }
    }
}

///////////////////////////////////////////////////
//      Counter x-y position class methods       //
///////////////////////////////////////////////////
id_x_y::id_x_y(int id, bool valid)
{
    this->id = id;
    this->valid = valid;
};

void Update_list::init(Ricom_kernel kernel, int nx_ricom, int ny_ricom)
{
    this->nx = nx_ricom;
    this->ny = ny_ricom;
    this->kernel = kernel;

    ids.resize(kernel.k_area);
    int idul = 0;
    for (int idy = -kernel.kernel_size; idy <= kernel.kernel_size; idy++)
    {
        for (int idx = -kernel.kernel_size; idx <= kernel.kernel_size; idx++)
        {
            ids[idul] = {(idy * nx + idx), false};
            idul++;
        }
    }
}

void Update_list::shift(id_x_y &res, int id, int shift)
{  
    int id_sft = ids[id].id + shift;
    int y = id_sft / nx;
    int x = id_sft % nx;
    res = {id_sft, y >= 0 && y < ny && x < nx && x >= 0};
}

////////////////////////////////////////////////
//               SDL plotting                 //
////////////////////////////////////////////////
void Ricom::init_surface()
{
    // Creating SDL Surface (holding the ricom image in CPU memory)
    srf_ricom = SDL_CreateRGBSurface(0, nx, ny, 32, 0, 0, 0, 0);
    if (srf_ricom == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    // SDL surface for STEM image
    srf_stem = SDL_CreateRGBSurface(0, nx, ny, 32, 0, 0, 0, 0);
    if (srf_stem == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    // SDL surface for CBED image
    cbed_log.assign(camera.nx_cam*camera.ny_cam, 0.0);
    srf_cbed = SDL_CreateRGBSurface(0, camera.nx_cam, camera.ny_cam, 32, 0, 0, 0, 0);
    if (srf_cbed == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Draw a pixel on the surface at (x, y) for a given colormap
void Ricom::draw_pixel(SDL_Surface *surface, int x, int y, float val, int col_map)
{
    cmap::Color c = cmap::GetColor(val, cmap::ColormapType(col_map));
    Uint32 px = SDL_MapRGB(surface->format, (Uint8)(c.r() * 255), (Uint8)(c.g() * 255), (Uint8)(c.b() * 255));
    Uint32 *const target_pixel = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
    *target_pixel = px;
}

////////////////////////////////////////////////
//     RICOM class method implementations     //
////////////////////////////////////////////////
Ricom::Ricom() : stem_max(-FLT_MAX), stem_min(FLT_MAX),
                 update_list(),
                 ricom_max(-FLT_MAX), ricom_min(FLT_MAX),
                 cbed_log(),
                 ricom_mutex(), stem_mutex(), counter_mutex(),
                 socket(), file_path(""),
                 camera(),
                 mode(RICOM::FILE),
                 b_print2file(false),
                 redraw_interval(50),
                 last_y(0),
                 p_prog_mon(nullptr),
                 b_busy(false),
                 update_dose_lowbound(6),
                 update_offset(true),
                 b_vSTEM(false), b_plot_cbed(true), b_plot2SDL(false),
                 b_recompute_detector(false), b_recompute_kernel(false),
                 detector(256, 256),
                 kernel(),
                 offset{127.5, 127.5}, com_public{0.0, 0.0},
                 com_map_x(), com_map_y(),
                 ricom_data(),
                 stem_data(),
                 nx(256), ny(256), nxy(0),
                 rep(1), fr_total(0),
                 skip_row(1), skip_img(0),
                 n_threads(1), queue_size(64),
                 fr_freq(0.0), fr_count(0.0), fr_count_total(0.0),
                 rescale_ricom(false), rescale_stem(false),
                 rc_quit(false),
                 srf_ricom(NULL), ricom_cmap(9),
                 srf_stem(NULL), stem_cmap(9),
                 srf_cbed(NULL), cbed_cmap(5)
{
    n_threads_max = std::thread::hardware_concurrency();
}

Ricom::~Ricom(){};

// Change the endianness of the incoming data
template <typename T>
void Ricom::swap_endianess(T &val)
{
    if (val > 0 && camera.swap_endian)
    {
        switch (sizeof(T))
        {
        case 2:
            val = (val >> 8) | ((val & 0xff) << 8);
            break;
        case 4:
            val = (val >> 24) | ((val & 0xff) << 24) | ((val & 0xff00) << 8) | ((val & 0xff0000) >> 8);
        default:
            break;
        }
    }
}

// Compute the centre of mass
template <typename T>
void Ricom::com(std::vector<T> *data, std::array<float, 2> &com, std::atomic<int> *dose_sum)
{
    float dose = 0;
    T px;
    size_t sum_x_temp;
    std::vector<size_t> sum_x(camera.nx_cam);
    std::vector<size_t> sum_y(camera.ny_cam);
    sum_x.assign(camera.nx_cam, 0);
    sum_y.assign(camera.ny_cam, 0);
    com = {0.0, 0.0};

    size_t y_nx;
    for (int idy = 0; idy < camera.ny_cam; idy++)
    {
        y_nx = idy * camera.nx_cam;
        sum_x_temp = 0;
        for (int idx = 0; idx < camera.nx_cam; idx++)
        {
            px = data->data()[y_nx + idx];
            swap_endianess(px);
            sum_x_temp += px;
            sum_y[idx] += px;
        }
        sum_x[idy] = sum_x_temp;
        dose += sum_x_temp;
    }

    if (dose > 0)
    {
        for (int i = 0; i < camera.nx_cam; i++)
        {
            com[0] += sum_x[i] * camera.v[i];
        }
        for (int i = 0; i < camera.ny_cam; i++)
        {
            com[1] += sum_y[i] * camera.u[i];
        }
        for (int i = 0; i < 2; i++)
        {
            com[i] = (com[i] / dose);
        }
        dose_sum[0] += (int)dose;
    }
}

// Compute STEM signal
template <typename T>
void Ricom::stem(std::vector<T> *data, size_t id_stem)
{
    T px;
    size_t stem_temp = 0;
    for (size_t id : detector.id_list)
    {
        px = (*data)[id];
        if (px > 0)
        {
            swap_endianess(px);
            stem_temp += px;
            if (stem_temp > stem_max)
            {
                stem_max = stem_temp;
                rescale_stem = true;
            }
            if (stem_temp < stem_min)
            {
                stem_min = stem_temp;
                rescale_stem = true;
            }
        }
    }
    stem_data[id_stem] = stem_temp;
}

// Integrate COM around position x,y
void Ricom::icom(std::array<float, 2> *com, int x, int y)
{
    float com_x = (*com)[0] - offset[0];
    float com_y = (*com)[1] - offset[1];
    id_x_y idr;
    int idc = x + y * nx;

    for (int id = 0; id < kernel.k_area; id++)
    {
        update_list.shift(idr, id, idc);
        if (idr.valid)
        {
            ricom_data[idr.id] += com_x * kernel.kernel_x[id] + com_y * kernel.kernel_y[id];
        }
    }
    if (ricom_data[idc] > ricom_max)
    {
        ricom_max = ricom_data[idc];
        rescale_ricom = true;
    }
    if (ricom_data[idc] < ricom_min)
    {
        ricom_min = ricom_data[idc];
        rescale_ricom = true;
    }
}

// Integrate COM around position x,y
void Ricom::icom(std::array<float, 2> com, int x, int y)
{
    float com_x = com[0] - offset[0];
    float com_y = com[1] - offset[1];
    id_x_y idr;
    int idc = x + y * nx;

    for (int id = 0; id < kernel.k_area; id++)
    {
        update_list.shift(idr, id, idc);
        if (idr.valid)
        {
            ricom_data[idr.id] += com_x * kernel.kernel_x[id] + com_y * kernel.kernel_y[id];
        }
    }
    if (ricom_data[idc] > ricom_max)
    {
        ricom_max = ricom_data[idc];
        rescale_ricom = true;
    }
    if (ricom_data[idc] < ricom_min)
    {
        ricom_min = ricom_data[idc];
        rescale_ricom = true;
    }
}

// Redraws the entire ricom image
void Ricom::draw_ricom_image()
{
    std::lock_guard<std::mutex> lock(ricom_mutex);
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_ricom_pixel(x, y);
        }
    }
}

// Redraws the ricom image from line y0 to line ye
void Ricom::draw_ricom_image(int y0, int ye)
{
    std::lock_guard<std::mutex> lock(ricom_mutex);
    for (int y = y0; y <= ye; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_ricom_pixel(x, y);
        }
    }
}

// set pixel in ricom image srf_ricom at location idx, idy to value corresponding
// value in ricom_data array
void Ricom::set_ricom_pixel(int idx, int idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = (ricom_data[idr] - ricom_min) / (ricom_max - ricom_min);

    // Update pixel at location
    draw_pixel(srf_ricom, idx, idy, val, ricom_cmap);
}

// Redraws the entire stem image
void Ricom::draw_stem_image()
{
    std::lock_guard<std::mutex> lock(stem_mutex);
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_stem_pixel(x, y);
        }
    }
}

// Redraws the stem image from line y0 to line ye
void Ricom::draw_stem_image(int y0, int ye)
{
    std::lock_guard<std::mutex> lock(stem_mutex);
    for (int y = y0; y <= ye; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_stem_pixel(x, y);
        }
    }
}

// set pixel in stem image srf_stem at location idx, idy to value corresponding
// value in stem_data array
void Ricom::set_stem_pixel(size_t idx, size_t idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = (stem_data[idr] - stem_min) / (stem_max - stem_min);

    // Update pixel at location
    draw_pixel(srf_stem, idx, idy, val, stem_cmap);
}

// Draw a CBED in log-scale to the SDL surface srf_cbed
template <typename T>
void Ricom::plot_cbed(std::vector<T> *cbed_data)
{
    float v_min = INFINITY;
    float v_max = 0.0;
    float vl_f;

    for (size_t id = 0; id < (*cbed_data).size(); id++)
    {
        T vl = (*cbed_data)[id];
        swap_endianess(vl);
        vl_f = log(1.0 + (float)vl);
        if (vl_f > v_max)
        {
            v_max = vl_f;
        }
        if (vl_f < v_min)
        {
            v_min = vl_f;
        }
        cbed_log[id] = vl_f;
    }

    int iy_t;
    float v_rng = v_max - v_min;
    for (int ix = 0; ix < camera.ny_cam; ix++)
    {
        iy_t = camera.v[ix] * camera.nx_cam;
        for (int iy = 0; iy < camera.nx_cam; iy++)
        {
            vl_f = cbed_log[iy_t + camera.u[iy]];
            float val = (vl_f - v_min) / v_rng;
            draw_pixel(srf_cbed, ix, iy, val, cbed_cmap);
        }
    }
}

// Rescales the images according to updated min and max values
// and recomputes the Kernel if settings changed
inline void Ricom::rescales_recomputes()
{
    if (b_recompute_detector)
    {
        detector.compute_detector(offset, camera.nx_cam, camera.ny_cam);
        b_recompute_detector = false;
    }
    if (b_recompute_kernel)
    {
        kernel.compute_kernel();
        update_list.init(kernel, nx, ny);
        b_recompute_kernel = false;
    }

    if (rescale_ricom)
    {
        rescale_ricom = false;
    };
    if (b_vSTEM)
    {
        if (rescale_stem)
        {
            rescale_stem = false;
        };
    };
}

// Skip n frames
template <typename T, class CameraInterface>
void Ricom::skip_frames(int n_skip, std::vector<T> &data, CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> *camera_fr)
{
    for (int si = 0; si < n_skip; si++)
    {
        camera_fr->read_frame(data, true);
    }
}

// Compute COM and iCOM for a frame (copy data)
template <typename T>
void Ricom::com_icom(std::vector<T> data, int ix, int iy, std::atomic<int> *dose_sum, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon)
{
    std::vector<T> *data_ptr = &data;

    std::array<float, 2> com_xy = {0.0, 0.0};
    com<T>(data_ptr, com_xy, dose_sum);
    icom(com_xy, ix, iy);

    if (b_vSTEM)
    {
        stem(data_ptr, iy * nx + ix);
    }

    com_xy_sum->at(0) += com_xy[0];
    com_xy_sum->at(1) += com_xy[1];

    int id = iy * nx + ix;
    com_map_x[id] = com_xy[0];
    com_map_y[id] = com_xy[1];

    counter_mutex.lock();
    ++(*p_prog_mon);
    fr_count = p_prog_mon->fr_count;
    if (p_prog_mon->report_set)
    {
        update_surfaces(iy, data_ptr);
        last_y = iy;
        fr_freq = p_prog_mon->fr_freq;
        rescales_recomputes();
        for (int i = 0; i < 2; i++)
        {
            com_public[i] = com_xy_sum->at(i) / p_prog_mon->fr_count_i;
            com_xy_sum->at(i) = 0;
        }
        p_prog_mon->reset_flags();
    }
    counter_mutex.unlock();
}

// Compute COM and iCOM for a frame (reference data)
template <typename T>
void Ricom::com_icom(std::vector<T> *data_ptr, int ix, int iy, std::atomic<int> *dose_sum, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon)
{
    std::array<float, 2> com_xy = {0.0, 0.0};
    com<T>(data_ptr, com_xy, dose_sum);
    icom(com_xy, ix, iy);

    if (b_vSTEM)
    {
        stem(data_ptr, iy * nx + ix);
    }

    com_xy_sum->at(0) += com_xy[0];
    com_xy_sum->at(1) += com_xy[1];

    int id = iy * nx + ix;
    ++(*p_prog_mon);
    com_map_x[id] = com_xy[0];
    com_map_y[id] = com_xy[1];
    fr_count = p_prog_mon->fr_count;
    if (p_prog_mon->report_set)
    {
        update_surfaces(iy, data_ptr);
        last_y = iy;
        fr_freq = p_prog_mon->fr_freq;
        rescales_recomputes();
        for (int i = 0; i < 2; i++)
        {
            com_public[i] = com_xy_sum->at(i) / p_prog_mon->fr_count_i;
            com_xy_sum->at(i) = 0;
        }
        p_prog_mon->reset_flags();
    }
}

// Process FRAME_BASED camera data
template <typename T, class CameraInterface>
void Ricom::process_data(CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> *camera_spec)
{
    // Memory allocation
    std::vector<T> data(camera_spec->nx_cam * camera_spec->ny_cam);
    std::vector<T> *p_data = &data;
    BoundedThreadPool pool;

    // Start Thread Pool
    if (n_threads > 1)
        pool.init(n_threads, queue_size);

    std::atomic<int> dose_sum = 0;
    std::atomic<int> *p_dose_sum = &dose_sum;

    std::array<float, 2> com_xy_sum = {0.0, 0.0};
    std::array<float, 2> *p_com_xy_sum = &com_xy_sum;

    // Initialize ProgressMonitor Object
    ProgressMonitor prog_mon(fr_total, !b_print2file, redraw_interval);
    p_prog_mon = &prog_mon;

    for (int ir = 0; ir < rep; ir++)
    {
        reinit_vectors_limits();
        for (int iy = 0; iy < ny; iy++)
        {
            for (int ix = 0; ix < nx; ix++)
            {
                camera_spec->read_frame(data, !p_prog_mon->first_frame);
                p_prog_mon->first_frame = false;
                if (n_threads > 1)
                {
                    pool.push_task([=]
                                   { com_icom<T>(data, ix, iy, p_dose_sum, p_com_xy_sum, p_prog_mon); });
                }
                else
                {
                    com_icom<T>(p_data, ix, iy, p_dose_sum, p_com_xy_sum, p_prog_mon);
                }

                if (rc_quit)
                {
                    pool.wait_for_completion();
                    p_prog_mon = nullptr;
                    return;
                };
            }
            skip_frames(skip_row, data, camera_spec);
        }
        skip_frames(skip_img, data, camera_spec);

        if (n_threads > 1)
            pool.wait_for_completion();

        if (update_offset && dose_sum > pow(10.0, update_dose_lowbound))
        {
            offset[0] = com_public[0];
            offset[1] = com_public[1];
            dose_sum = 0;
        }
    }
    p_prog_mon = nullptr;
}

template <typename T>
void Ricom::update_surfaces(int iy, std::vector<T> *p_frame)
{
    if (b_plot2SDL)
    {
        draw_ricom_image((std::max)(0, last_y - kernel.kernel_size), (std::min)(iy + kernel.kernel_size, ny - 1));
        if (b_vSTEM)
        {
            draw_stem_image(last_y, iy);
        }
    }
    if (b_plot_cbed)
    {
        plot_cbed(p_frame);
    }
}
// Process EVENT_BASED camera data
template <class CameraInterface>
void Ricom::process_data(CAMERA::Camera<CameraInterface, CAMERA::EVENT_BASED> *camera_spec)
{
    // Memory allocation
    std::vector<size_t> dose_map(nxy);
    std::vector<size_t> sumx_map(nxy);
    std::vector<size_t> sumy_map(nxy);
    std::vector<uint16_t> frame(camera_spec->nx_cam * camera_spec->ny_cam);
    std::vector<uint16_t> *p_frame = &frame;

    BoundedThreadPool pool;

    // Start Thread Pool
    if (n_threads > 1)
        pool.init(n_threads, queue_size);

    std::array<float, 2> com_xy = {0.0, 0.0};
    std::array<float, 2> *p_com_xy = &com_xy;
    std::array<float, 2> com_xy_sum = {0.0, 0.0};
    int dose_sum = 0;
    int idxx = 0;
    int ix = 0;
    int iy = 0;

    size_t img_num = 0;
    size_t first_frame = img_num * nxy;
    size_t end_frame = (img_num + 1) * nxy;
    size_t fr_total_u = (size_t)fr_total;

    ProgressMonitor prog_mon(fr_total, !b_print2file, redraw_interval);
    p_prog_mon = &prog_mon;

    dose_map.assign(nxy, 0);
    sumx_map.assign(nxy, 0);
    sumy_map.assign(nxy, 0);
    reinit_vectors_limits();

    while (true)
    {
        // process two frames before the probe position to avoid toa problem
        idxx = prog_mon.fr_count - nxy * img_num - 2;

        ++prog_mon;
        fr_count = prog_mon.fr_count;

        if (prog_mon.report_set || b_vSTEM)
        {
            camera_spec->read_frame_com_cbed(prog_mon.fr_count, dose_map, sumx_map, sumy_map, frame, first_frame, end_frame);
        }
        else
        {
            camera_spec->read_frame_com(prog_mon.fr_count, dose_map, sumx_map, sumy_map, first_frame, end_frame);
        }

        if (idxx >= 0)
        {
            if (dose_map[idxx] == 0)
            {
                com_xy[0] = offset[0];
                com_xy[1] = offset[1];
            }
            else
            {
                com_xy[0] = sumx_map[idxx] / dose_map[idxx];
                com_xy[1] = sumy_map[idxx] / dose_map[idxx];
            }
            com_map_x[idxx] = com_xy[0];
            com_map_y[idxx] = com_xy[1];
            com_xy_sum[0] += com_xy[0];
            com_xy_sum[1] += com_xy[1];
            dose_sum += dose_map[idxx];

            ix = idxx % nx;
            iy = floor(idxx / nx);

            if (n_threads > 1)
            {
                pool.push_task([=]
                               { icom(com_xy, ix, iy); });
            }
            else
            {
                icom(p_com_xy, ix, iy);
            }

            if (b_vSTEM)
            {
                stem(p_frame, iy * nx + ix);
            }
        }
    
    if (prog_mon.report_set)
    {
        update_surfaces(iy, p_frame);
        plot_cbed(p_frame);
        fr_freq = prog_mon.fr_freq;
        rescales_recomputes();
        for (int i = 0; i < 2; i++)
        {
            com_public[i] = com_xy_sum[i] / prog_mon.fr_count_i;
            com_xy_sum[i] = 0;
        }
        prog_mon.reset_flags();
        last_y = iy;
    }

    if (prog_mon.fr_count >= end_frame)
    {
        if (prog_mon.fr_count != fr_total_u)
        {
            img_num++;
            first_frame = img_num * nxy;
            end_frame = (img_num + 1) * nxy;
            reinit_vectors_limits();
            dose_map.assign(nxy, 0);
            sumx_map.assign(nxy, 0);
            sumy_map.assign(nxy, 0);
        }

        if (update_offset && dose_sum > pow(10.0, update_dose_lowbound))
        {
            offset[0] = com_public[0];
            offset[1] = com_public[1];
            dose_sum = 0;
        }
    }

    if (prog_mon.fr_count == fr_total_u || rc_quit)
    {
        pool.wait_for_completion();
        p_prog_mon = nullptr;
        return;
    }
}
p_prog_mon = nullptr;
}

// Entrance function for Ricom_reconstructinon
template <class CameraInterface>
void Ricom::run_reconstruction(RICOM::modes mode)
{
    reset();
    this->mode = mode;
    b_busy = true;
    // Initializations
    nxy = nx * ny;
    fr_total = nxy * rep;
    fr_count = 0;

    camera.init_uv_default();
    init_surface();

    if (b_vSTEM)
    {
        detector.compute_detector(offset, camera.nx_cam, camera.ny_cam);
    }

    // Compute the integration Kenel
    kernel.compute_kernel();

    // Allocate the ricom image and COM arrays
    ricom_data.resize(nxy);
    update_list.init(kernel, nx, ny);
    com_map_x.resize(nxy);
    com_map_y.resize(nxy);
    stem_data.resize(nxy);

    // Run camera dependent pipeline
    // Implementations are in the Interface Wrappers (src/cameras/XXXWrapper.cpp), but essentially
    // they run Ricom::process_data<FRAME_BASED>() or Ricom::process_data<EVENT_BASED>()
    switch (camera.type)
    {
    case CAMERA::FRAME_BASED:
    {
        CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> camera_interface(camera);
        camera_interface.run(this);
        break;
    }
    case CAMERA::EVENT_BASED:
    {
        CAMERA::Camera<CameraInterface, CAMERA::EVENT_BASED> camera_interface(camera);
        camera_interface.run(this);
        break;
    }
    }
    b_busy = false;
    std::cout << std::endl
              << "Reconstruction finished successfully." << std::endl;
}

// Template specializations, necessary to avoid linker error
template void Ricom::run_reconstruction<MerlinInterface>(RICOM::modes);
template void Ricom::run_reconstruction<TimepixInterface>(RICOM::modes);
template void Ricom::process_data<uint8_t>(CAMERA::Camera<MerlinInterface, CAMERA::FRAME_BASED> *camera_spec);
template void Ricom::process_data<uint16_t>(CAMERA::Camera<MerlinInterface, CAMERA::FRAME_BASED> *camera_spec);
template void Ricom::process_data(CAMERA::Camera<TimepixInterface, CAMERA::EVENT_BASED> *camera_spec);

// Helper functions
void Ricom::reset_limits()
{
    ricom_max = -FLT_MAX;
    ricom_min = FLT_MAX;
    stem_max = -FLT_MAX;
    stem_min = FLT_MAX;
}

void Ricom::reinit_vectors_limits()
{
    ricom_data.assign(nxy, 0);
    stem_data.assign(nxy, 0);
    com_map_x.assign(nxy, 0);
    com_map_y.assign(nxy, 0);
    last_y = 0;
    reset_limits();
}

void Ricom::reset()
{
    rc_quit = false;
    fr_freq = 0;
    reinit_vectors_limits();
}