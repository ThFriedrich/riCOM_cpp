
#ifdef _WIN32
#include <io.h>
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
#include "ProgressMonitor.hpp"

namespace chc = std::chrono;
namespace cmap = tinycolormap;

////////////////////////////////////////////////
// RICOM Kernel class method implementations  //
////////////////////////////////////////////////

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

    // Compute the kernel
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
        absorb_filter();
    }
}

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

void Ricom_kernel::absorb_filter()
{
    int k_width_sym = kernel_size * 2 + 1;
    std::vector<int> map = fftshift_map(k_width_sym, k_width_sym);
    std::complex<double> *k_x = new std::complex<double>[k_area];
    std::complex<double> *k_y = new std::complex<double>[k_area];
    std::complex<double> *k_x_f = new std::complex<double>[k_area];
    std::complex<double> *k_y_f = new std::complex<double>[k_area];

    fftw_plan px, py, ipx, ipy;

    for (int id = 0; id < k_area; id++)
    {
        k_x[id + map[id]] = {kernel_x[id], 0};
        k_y[id + map[id]] = {kernel_y[id], 0};
    }

    px = fftw_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftw_complex *>(k_x),
                          reinterpret_cast<fftw_complex *>(k_x_f), FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(px);
    fftw_destroy_plan(px);

    py = fftw_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftw_complex *>(k_y),
                          reinterpret_cast<fftw_complex *>(k_y_f), FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(py);
    fftw_destroy_plan(py);

    for (int id = 0; id < k_area; id++)
    {
        if (kernel_filter[id + map[id]] == 0)
        {
            k_x_f[id] = {0, 0};
            k_y_f[id] = {0, 0};
        }
    }

    ipx = fftw_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftw_complex *>(k_x_f),
                           reinterpret_cast<fftw_complex *>(k_x), FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(ipx);
    fftw_destroy_plan(ipx);

    ipy = fftw_plan_dft_2d(k_width_sym, k_width_sym, reinterpret_cast<fftw_complex *>(k_y_f),
                           reinterpret_cast<fftw_complex *>(k_y), FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(ipy);
    fftw_destroy_plan(ipy);

    for (int id = 0; id < k_area; id++)
    {
        kernel_x[id] = k_x[id + map[id]].real();
        kernel_y[id] = k_y[id + map[id]].real();
    }
}

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
        shift_x = 1;
    }
    if (y % 2 == 0)
    {
        shift_y = 0;
    }
    else
    {
        shift_y = 1;
    }

    int q1 = (center_y + shift_y) * x + center_x + shift_x;
    int q2 = (center_y + shift_y) * x - center_x;
    int q3 = -1 * center_y * x + center_x + shift_x;
    int q4 = -1 * center_y * x - center_x;

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

///////////////////////////////////////////////////
//  RICOM detector class method implementations  //
///////////////////////////////////////////////////

void Ricom_detector::compute_detector()
{
    float rad_in2 = pow(radius[0], 2), rad_out2 = pow(radius[1], 2);
    float d2;
    id_list.clear();

    for (size_t iy = 0; iy < ny_cam; iy++)
    {
        for (size_t ix = 0; ix < nx_cam; ix++)
        {
            d2 = pow((float)ix - offset[0], 2) + pow((float)iy - offset[1], 2);
            if (d2 > rad_in2 && d2 <= rad_out2)
            {
                id_list.push_back(iy * nx_cam + ix);
            }
        }
    }
}

void Ricom_detector::compute_detector(std::array<float, 2> &offset)
{
    float rad_in2 = pow(radius[0], 2), rad_out2 = pow(radius[1], 2);
    float d2;
    id_list.clear();

    for (size_t iy = 0; iy < ny_cam; iy++)
    {
        for (size_t ix = 0; ix < nx_cam; ix++)
        {
            d2 = pow((float)ix - offset[0], 2) + pow((float)iy - offset[1], 2);
            if (d2 > rad_in2 && d2 <= rad_out2)
            {
                id_list.push_back(iy * nx_cam + ix);
            }
        }
    }
}

////////////////////////////////////////////////
//               SDL plotting                 //
////////////////////////////////////////////////
void Ricom::init_surface()
{
    // Creating Surface (holding the ricom image in CPU memory)
    srf_ricom = SDL_CreateRGBSurface(0, nx, ny, 32, 0, 0, 0, 0);
    if (srf_ricom == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    srf_stem = SDL_CreateRGBSurface(0, nx, ny, 32, 0, 0, 0, 0);
    if (srf_stem == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    srf_cbed = SDL_CreateRGBSurface(0, nx_merlin, ny_merlin, 32, 0, 0, 0, 0);
    if (srf_cbed == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
}

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
Ricom::Ricom() : stem_data(), stem_max(-FLT_MAX), stem_min(FLT_MAX), u(), v(), ricom_data(), update_list(), ricom_max(-FLT_MAX), ricom_min(FLT_MAX), ricom_mutex(), stem_mutex(), counter_mutex(), mode(RICOM::FILE), b_print2file(false), redraw_interval(20), update_dose_lowbound(6), update_offset(true), use_detector(false), b_recompute_detector(false), b_recompute_kernel(false), detector(), kernel(), offset{127.5, 127.5}, com_public{0.0, 0.0}, com_map_x(), com_map_y(), detector_type(RICOM::MERLIN), nx(257), ny(256), nxy(0), rep(1), fr_total(0), skip_row(0), skip_img(0), n_threads(1), queue_size(8), fr_freq(0.0), fr_count(0.0), fr_count_total(0.0), rescale_ricom(false), rescale_stem(false), rc_quit(false), srf_ricom(NULL), ricom_cmap(9), srf_stem(NULL), stem_cmap(9), srf_cbed(NULL), cbed_cmap(9)
{
    n_threads_max = std::thread::hardware_concurrency();
    n_threads = n_threads_max;
}

Ricom::~Ricom(){};

void Ricom::init_uv()
{
    u.assign(nx_merlin, 0);
    v.assign(nx_merlin, 0);

    if (b_raw)
    // raw format: pixel sequence flipped every 64 bit
    {
        size_t num_per_flip = 64;
        switch (depth)
        {
        case 1:
            num_per_flip = 64;
            break;
        case 6:
            num_per_flip = 8;
            break;
        case 12:
            num_per_flip = 4;
            break;
        }
        size_t cnt = 0;
        for (size_t i = 0; i < (nx_merlin / num_per_flip); i++)
        {
            for (size_t j = (num_per_flip * (i + 1)); j > num_per_flip * i; j--)
            {
                u[cnt] = (j - 1);
                cnt++;
            }
        }
    }
    else
    {
        for (int i = 0; i < nx_merlin; i++)
        {
            u[i] = i;
        }
    }

    for (int i = 0; i < ny_merlin; i++)
    {
        v[i] = i;
    }
}

template <typename T>
void Ricom::com(std::vector<T> *data, std::array<float, 2> &com, std::atomic<int> *dose_sum)
{
    float dose = 0;
    T px;
    size_t sum_x_temp;
    std::array<size_t, 256> sum_x = {0};
    std::array<size_t, 256> sum_y = {0};
    com[0] = 0;
    com[1] = 0;

    size_t y_nx;
    for (int idy = 0; idy < ny_merlin; idy++)
    {
        y_nx = idy * nx_merlin;
        sum_x_temp = 0;
        for (int idx = 0; idx < nx_merlin; idx++)
        {
            px = (*data)[y_nx + idx];
            byte_swap(px);
            sum_x_temp += px;
            sum_y[idx] += px;
        }

        sum_x[idy] = sum_x_temp;
        dose += sum_x_temp;
    }

    if (dose > 0)
    {
        for (int i = 0; i < nx_merlin; i++)
        {
            com[0] += sum_x[i] * v[i];
        }
        for (int i = 0; i < ny_merlin; i++)
        {
            com[1] += sum_y[i] * u[i];
        }
        for (int i = 0; i < 2; i++)
        {
            com[i] = (com[i] / dose);
        }
        dose_sum[0] += (int)dose;
    }
}

template <typename T>
void Ricom::stem(std::vector<T> *data, size_t id_stem)
{
    T px;
    size_t stem_temp = 0;
    for (size_t id : detector.id_list)
    {
        px = data->at(id);
        byte_swap(px);
        stem_temp += px;
    }
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
    stem_data[id_stem] = stem_temp;
}

void Ricom::icom(std::array<float, 2> &com, int x, int y)
{
    float com_x = com[0] - offset[0];
    float com_y = com[1] - offset[1];
    id_x_y idr;
    int idc = x + y * nx;

    for (int id = 0; id < kernel.k_area; id++)
    {
        idr = update_list[id] + idc;
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

std::vector<id_x_y> Ricom::calculate_update_list()
{
    std::vector<id_x_y> ul(kernel.k_area);
    int idul = 0;
    for (int idy = -kernel.kernel_size; idy <= kernel.kernel_size; idy++)
    {
        for (int idx = -kernel.kernel_size; idx <= kernel.kernel_size; idx++)
        {
            ul[idul] = {(idy * nx + idx), idx, idy, nx, ny, false};
            idul++;
        }
    }
    return ul;
}

void Ricom::draw_ricom_image()
{
    std::scoped_lock lock(ricom_mutex);
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_ricom_pixel(x, y);
        }
    }
}

void Ricom::draw_ricom_image(int y0, int ye)
{
    std::scoped_lock lock(ricom_mutex);
    for (int y = y0; y < ye; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_ricom_pixel(x, y);
        }
    }
}

void Ricom::set_ricom_pixel(int idx, int idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = (ricom_data[idr] - ricom_min) / (ricom_max - ricom_min);

    // Update pixel at location
    draw_pixel(srf_ricom, idx, idy, val, ricom_cmap);
}

void Ricom::draw_stem_image()
{
    std::scoped_lock lock(stem_mutex);
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_stem_pixel(x, y);
        }
    }
}

void Ricom::draw_stem_image(int y0, int ye)
{
    std::scoped_lock lock(stem_mutex);
    for (int y = y0; y < ye; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_stem_pixel(x, y);
        }
    }
}

void Ricom::set_stem_pixel(size_t idx, size_t idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = (stem_data[idr] - stem_min) / (stem_max - stem_min);

    // Update pixel at location
    draw_pixel(srf_stem, idx, idy, val, stem_cmap);
}

template <typename T>
void Ricom::plot_cbed(std::vector<T> *cbed_data)
{
    float v_min = INFINITY;
    float v_max = 0.0;
    float vl_f;
    for (size_t id = 0; id < (*cbed_data).size(); id++)
    {
        T vl = (*cbed_data)[id];
        byte_swap(vl);
        vl_f = log(1.0 + (float)vl);
        if (vl_f > v_max)
        {
            v_max = vl_f;
        }
        if (vl_f < v_min)
        {
            v_min = vl_f;
        }
    }

    int iy_t;
    for (int ix = 0; ix < ny_merlin; ix++)
    {
        iy_t = v[ix] * nx_merlin;
        for (int iy = 0; iy < nx_merlin; iy++)
        {
            T vl = (*cbed_data)[iy_t + u[iy]];
            byte_swap(vl);
            vl_f = log(1 + (float)vl);
            float val = (vl_f - v_min) / (v_max - v_min);
            draw_pixel(srf_cbed, ix, iy, val, cbed_cmap);
        }
    }
}

inline void Ricom::rescales_recomputes()
{
    if (b_recompute_detector)
    {
        detector.compute_detector(offset);
        b_recompute_detector = false;
    }
    if (b_recompute_kernel)
    {
        kernel.compute_kernel();
        update_list = calculate_update_list();
        b_recompute_kernel = false;
    }

    if (rescale_ricom)
    {
        rescale_ricom = false;
        draw_ricom_image();
    };
    if (use_detector)
    {
        if (rescale_stem)
        {
            rescale_stem = false;
            draw_stem_image();
        };
    };
}

template <typename T>
inline void Ricom::skip_frames(int n_skip, std::vector<T> &data)
{
    for (int si = 0; si < n_skip; si++)
    {
        read_data<T>(data, true);
    }
}

template <typename T>
void Ricom::com_icom(std::vector<T> data, int ix, int iy, std::atomic<int> *dose_sum, std::array<std::atomic<float>, 2> *com_xy_sum, ProgressMonitor *p_prog_mon)
{
    std::vector<T> *data_ptr = &data;

    std::array<float, 2> com_xy = {0.0, 0.0};
    com<T>(data_ptr, com_xy, dose_sum);
    icom(com_xy, ix, iy);

    if (use_detector)
    {
        stem(data_ptr, iy * nx + ix);
    }

    com_xy_sum->at(0) += com_xy[0];
    com_xy_sum->at(1) += com_xy[1];

    int id = iy * nx + ix;
    counter_mutex.lock();
    ++(*p_prog_mon);
    com_map_x[id] = com_xy[0];
    com_map_y[id] = com_xy[1];
    fr_count = p_prog_mon->fr_count;
    if (p_prog_mon->report_set)
    {
        draw_ricom_image((std::max)(0, last_y - kernel.kernel_size), iy);
        if (use_detector)
        {
            draw_stem_image(last_y, iy);
        }
        last_y = iy;
        fr_freq = p_prog_mon->fr_freq;
        rescales_recomputes();
        plot_cbed<T>(data_ptr);
        for (int i = 0; i < 2; i++)
        {
            com_public[i] = com_xy_sum->at(i) / p_prog_mon->fr_count_i;
            com_xy_sum->at(i) = 0;
        }
        p_prog_mon->reset_flags();
    }
    counter_mutex.unlock();
}


template <typename T>
void Ricom::com_icom(std::vector<T> *data_ptr, int ix, int iy, std::atomic<int> *dose_sum, std::array<std::atomic<float>, 2> *com_xy_sum, ProgressMonitor *p_prog_mon)
{
    std::array<float, 2> com_xy = {0.0, 0.0};
    com<T>(data_ptr, com_xy, dose_sum);
    icom(com_xy, ix, iy);

    if (use_detector)
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
        draw_ricom_image((std::max)(0, last_y - kernel.kernel_size), iy);
        if (use_detector)
        {
            draw_stem_image(last_y, iy);
        }
        last_y = iy;
        fr_freq = p_prog_mon->fr_freq;
        rescales_recomputes();
        plot_cbed<T>(data_ptr);
        for (int i = 0; i < 2; i++)
        {
            com_public[i] = com_xy_sum->at(i) / p_prog_mon->fr_count_i;
            com_xy_sum->at(i) = 0;
        }
        p_prog_mon->reset_flags();
    }
}

template <typename T>
void Ricom::process_frames()
{
    // Memory allocation
    std::vector<T> data(ds_merlin);
    std::vector<T> *p_data = &data;
    BoundedThreadPool pool;

    // Start Thread Pool
    if (n_threads > 1)
        pool.init(n_threads, queue_size);   

    std::atomic<int> dose_sum = 0;
    std::atomic<int> *p_dose_sum = &dose_sum;

    std::array<std::atomic<float>, 2> com_xy_sum = {0.0, 0.0};
    std::array<std::atomic<float>, 2> *p_com_xy_sum = &com_xy_sum;

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
                read_data<T>(data, !p_prog_mon->first_frame);
                p_prog_mon->first_frame = false;
                if (n_threads > 1)
                {
                    pool.push_task([=, this]
                               { com_icom<T>(data, ix, iy, p_dose_sum, p_com_xy_sum, p_prog_mon); });
                }
                else
                {
                    com_icom<T>(p_data, ix, iy, p_dose_sum, p_com_xy_sum, p_prog_mon);  
                }
                
                if (rc_quit)
                {
                    pool.wait_for_completion();
                    return;
                };
            }
            skip_frames(skip_row, data);
        }
        skip_frames(skip_img, data);

        if (n_threads > 1)
            pool.wait_for_completion();

        if (mode == RICOM::modes::FILE)
        {
            reset_file();
        }

        if (update_offset && dose_sum > pow(10.0, update_dose_lowbound))
        {
            offset[0] = com_public[0];
            offset[1] = com_public[1];
            dose_sum = 0;
        }
    }
}

void Ricom::run_merlin()
{
    if (mode == RICOM::LIVE)
    {
        if (read_aquisition_header() == -1)
        {
            perror("Ricom::run_merlin() could not obtain aquisition_header");
            return;
        }
    }

    // Run the main loop
    if (read_head())
    {
        if (dtype == "U08")
        {
            process_frames<unsigned char>();
        }
        else if (dtype == "U16")
        {
            process_frames<unsigned short>();
        }
        else if (dtype == "R64")
        {
            b_raw = true;
            b_binary = false;
            switch (depth)
            {
            case 1:
                b_binary = true;
                process_frames<unsigned char>();
                break;
            case 6:
                process_frames<unsigned char>();
                break;
            case 12:
                process_frames<unsigned short>();
                break;
            default:
                break;
            }
        }
    }
    else
    {
        perror("Ricom::run_merlin() could not obtain aquisition_header");
    }
}

void Ricom::process_timepix_stream()
{
    // Memory allocation
    std::vector<size_t> dose_map(nxy);
    std::vector<size_t> sumx_map(nxy);
    std::vector<size_t> sumy_map(nxy);
    std::vector<size_t> comx_map(nxy);
    std::vector<size_t> comy_map(nxy);

    std::array<float, 2> com_xy = {0.0, 0.0};
    std::array<float, 2> com_xy_sum = {0.0, 0.0};
    int dose_sum = 0;
    int idx = 0;
    int idxx = 0;
    int ix;
    int iy;
    int img_num = 0;
    int first_frame = img_num * nxy;
    int end_frame = (img_num + 1) * nxy;
    bool b_while = true;

    // Initialize Progress bar
    // ProgressBar bar(fr_total, "kHz", !b_print2file);

    // Performance measurement
    auto start_perf = chc::high_resolution_clock::now();

    float fr = 0;          // Frequncy per frame
    size_t fr_count_i = 0; // Frame count in interval
    float fr_avg = 0;      // Average frequency
    size_t fr_count_a = 0; // Count measured points for averaging

    dose_map.assign(nxy, 0);
    sumx_map.assign(nxy, 0);
    sumy_map.assign(nxy, 0);
    comx_map.assign(nxy, 0);
    comy_map.assign(nxy, 0);
    ricom_data.assign(nxy, 0);
    stem_data.assign(nxy, 0);

    while (b_while)
    {
        read_com_ti(idx, dose_map, sumx_map, sumy_map, first_frame, end_frame);
        // std::cout << idx << std::endl;
        idxx = idx - nxy * img_num - 2;
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
            comx_map[idxx] = com_xy[0];
            comy_map[idxx] = com_xy[1];
            com_xy_sum[0] += com_xy[0];
            com_xy_sum[1] += com_xy[1];
            dose_sum += dose_map[idxx];

            ix = idxx % nx;
            iy = floor(idxx / nx);
            icom(com_xy, ix, iy); // process two frames before the probe position to avoid toa problem
        }

        fr_count_i++;
        auto mil_secs = std::chrono::duration_cast<RICOM::double_ms>(chc::high_resolution_clock::now() - start_perf).count();
        if (mil_secs > 500.0 || idx == fr_total) // ~50Hz for display
        {
            rescales_recomputes();
            std::cout << idx << " rescale'd" << std::endl;
            if (rc_quit)
            {
                return;
            }

            // plot_cbed<T>(data);
            fr = fr_count_i / mil_secs;
            fr_avg += fr;
            fr_count_a++;
            start_perf = chc::high_resolution_clock::now();
            fr_freq = fr_avg / fr_count_a;
            for (int i = 0; i < 2; i++)
            {
                com_public[i] = com_xy_sum[i] / fr_count_i;
                com_xy_sum[i] = 0;
            }
            fr_count_i = 0;
            // bar.Progressed(fr_count, fr_avg / fr_count_a);
            std::cout << idx << " progress'd" << std::endl;
        }
        if (idx >= end_frame && idx != fr_total)
        {
            img_num++;
            first_frame = img_num * nxy;
            end_frame = (img_num + 1) * nxy;
            reset_limits();
            dose_map.assign(nxy, 0);
            sumx_map.assign(nxy, 0);
            sumy_map.assign(nxy, 0);
            comx_map.assign(nxy, 0);
            comy_map.assign(nxy, 0);
            ricom_data.assign(nxy, 0);
            stem_data.assign(nxy, 0);
        }
        if (idx >= fr_total)
        {
            b_while = false;
            std::cout << "dose sum" << dose_sum << std::endl;
        }
    }
}

void Ricom::run_timepix()
{
    // Run the main loop
    // dwell_time = 6000;
    nx_timpix = 256;
    ny_timpix = 256;
    process_timepix_stream();
}

void Ricom::run()
{
    // Initializations
    nxy = nx * ny;
    fr_total = nxy * rep;
    fr_count = 0;

    init_surface();

    if (use_detector)
    {
        detector.compute_detector();
        stem_data.reserve(nxy);
    }

    // Compute the integration Kenel
    kernel.compute_kernel();

    // Allocate the ricom image
    ricom_data.reserve(nxy);
    update_list = calculate_update_list();
    com_map_x.reserve(nxy);
    com_map_y.reserve(nxy);

    init_uv();

    // Process the Data
    switch (detector_type)
    {
    case RICOM::MERLIN:
        merlin_init(mode);
        run_merlin();
        break;
    case RICOM::TIMEPIX:
        timepix_init(mode);
        run_timepix();
        break;
    default:
        break;
    }

    // Cleaning up
    switch (detector_type)
    {
    case RICOM::MERLIN:
        merlin_end();
        break;
    case RICOM::TIMEPIX:
        timepix_end();
        break;
    default:
        break;
    }
    std::cout << "All done." << std::endl;
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

void Ricom::reset_limits()
{
    ricom_max = -FLT_MAX;
    ricom_min = FLT_MAX;
    stem_max = -FLT_MAX;
    stem_min = FLT_MAX;
}

void Ricom::reset_file()
{
    mib_stream.clear();
    mib_stream.seekg(0, std::ios::beg);
}

void Ricom::reset()
{
    rc_quit = false;
    fr_freq = 0;
    b_binary = false;
    b_raw = false;
    reset_limits();
}