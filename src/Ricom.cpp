
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <thread>
#include <future>
#include "tinycolormap.hpp"

#include "Ricom.h"
#include "progress_bar.hpp"

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
void Ricom::init_surface(unsigned int width, unsigned int height)
{
    // Creating Surface (holding the ricom image in CPU memory)
    srf_ricom = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
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
        for (size_t i = 0; i < nx_merlin; i++)
        {
            u[i] = i;
        }
    }

    for (size_t i = 0; i < ny_merlin; i++)
    {
        v[i] = i;
    }
}

template <typename T>
void Ricom::com(std::vector<T> *data, std::array<float, 2> &com, int *dose_sum)
{
    float dose = 0;
    T px;
    size_t sum_x_temp;
    std::array<size_t, 256> sum_x = {0};
    std::array<size_t, 256> sum_y = {0};
    com[0] = 0;
    com[1] = 0;

    size_t y_nx;
    for (size_t idy = 0; idy < ny_merlin; idy++)
    {
        y_nx = idy * nx_merlin;
        sum_x_temp = 0;
        for (size_t idx = 0; idx < nx_merlin; idx++)
        {
            px = data->at(y_nx + idx);
            byte_swap(px);
            sum_x_temp += px;
            sum_y[idx] += px;
        }

        sum_x[idy] = sum_x_temp;
        dose += sum_x_temp;
    }

    if (dose > 0)
    {
        for (size_t i = 0; i < nx_merlin; i++)
        {
            com[0] += sum_x[i] * v[i];
        }
        for (size_t i = 0; i < ny_merlin; i++)
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

void Ricom::rescale_ricom_image()
{
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_ricom_pixel(x, y);
        }
    }
    rescale_ricom = false;
}

void Ricom::set_ricom_image_kernel(int ix, int iy)
{
    id_x_y idr;
    int idc = ix + iy * nx;

    for (int id = 0; id < kernel.k_area; id++)
    {
        idr = update_list[id] + idc;
        if (idr.valid)
        {
            set_ricom_pixel(idr);
        }
    }
}

void Ricom::set_ricom_pixel(id_x_y idr)
{
    // Update pixel at location [idr.x idr.y]
    float val = (ricom_data[idr.id] - ricom_min) / (ricom_max - ricom_min);
    ricom_mutex.lock();
    draw_pixel(srf_ricom, idr.x, idr.y, val, ricom_cmap);
    ricom_mutex.unlock();
}

void Ricom::set_ricom_pixel(int idx, int idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = (ricom_data[idr] - ricom_min) / (ricom_max - ricom_min);

    // Update pixel at location
    ricom_mutex.lock();
    draw_pixel(srf_ricom, idx, idy, val, ricom_cmap);
    ricom_mutex.unlock();
}

void Ricom::set_stem_pixel(size_t idx, size_t idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = (stem_data[idr] - stem_min) / (stem_max - stem_min);

    // Update pixel at location
    stem_mutex.lock();
    draw_pixel(srf_stem, idx, idy, val, stem_cmap);
    stem_mutex.unlock();
}

template <typename T>
void Ricom::plot_cbed(std::vector<T> cbed_data)
{
    float v_min = INFINITY;
    float v_max = 0.0;
    float vl_f;
    for (size_t id = 0; id < cbed_data.size(); id++)
    {
        T vl = cbed_data[id];
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

    size_t iy_t;
    for (size_t ix = 0; ix < ny_merlin; ix++)
    {
        iy_t = v[ix] * nx_merlin;
        for (size_t iy = 0; iy < nx_merlin; iy++)
        {
            T vl = cbed_data[iy_t + u[iy]];
            byte_swap(vl);
            vl_f = log(1 + (float)vl);
            float val = (vl_f - v_min) / (v_max - v_min);
            draw_pixel(srf_cbed, ix, iy, val, cbed_cmap);
        }
    }
}

void Ricom::rescale_stem_image()
{
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_stem_pixel(x, y);
        }
    }
    rescale_stem = false;
}

// For synchronization. Before recomputing kernel or detector all threads must
// finish their current computation, so that the kernel or detector isn't chaged
// half way through processing a frame. Could be avoided using mutex's on the
// kernel and detector objects, but this is easier and may be faster.
inline void get_futures(std::vector<std::future<void>> &futures)
{
    for (auto &f : futures)
    {
        if (f.valid())
        {
            f.get();
        }
    }
}

inline void Ricom::rescales_recomputes(std::vector<std::future<void>> &futures)
{
    if (rescale_ricom)
    {
        rescale_ricom_image();
    };
    if (use_detector)
    {
        if (rescale_stem)
        {
            rescale_stem_image();
        };
    };
    if (b_recompute_detector)
    {
        get_futures(futures);
        detector.compute_detector(offset);
        b_recompute_detector = false;
    }
    if (b_recompute_kernel)
    {
        get_futures(futures);
        kernel.compute_kernel();
        update_list = calculate_update_list();
        b_recompute_kernel = false;
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
        rescale_ricom_image();
    };
    if (use_detector)
    {
        if (rescale_stem)
        {
            rescale_stem_image();
        };
    };
}

template <typename T>
inline void Ricom::skip_frames(int n_skip, std::vector<T> &data)
{
    for (int si = 0; si < n_skip; si++)
    {
        if (fr_count < fr_total)
        {
            read_data<T>(data, true);
            fr_count++;
        }      
    }
}

template <typename T>
void Ricom::com_icom(std::vector<T> data, int ix, int iy, int *dose_sum, std::array<float, 2> *com_xy_sum)
{
    std::array<float, 2> com_xy = {0.0, 0.0};
    std::vector<T> *data_ptr = &data;

    com<T>(data_ptr, com_xy, dose_sum);
    icom(com_xy, ix, iy);
    set_ricom_image_kernel(ix, iy);
    if (use_detector)
    {
        stem(data_ptr, iy * nx + ix);
        set_stem_pixel(ix, iy);
    }

    int id = iy * nx + ix;
    counter_mutex.lock();
    com_xy_sum->at(0) += com_xy[0];
    com_xy_sum->at(1) += com_xy[1];
    com_map_x[id] = com_xy[0];
    com_map_y[id] = com_xy[1];
    fr_count++;
    counter_mutex.unlock();
}

template <typename T>
void Ricom::process_frames()
{
    // Memory allocation
    std::vector<T> data(ds_merlin);
    std::array<float, 2> com_xy_sum = {0.0, 0.0};
    int dose_sum = 0;

    // Initialize Progress bar
    ProgressBar bar(fr_total, "kHz", !b_print2file);

    // Performance measurement
    auto start_perf = chc::high_resolution_clock::now();

    float fr = 0;          // Frequncy per frame
    float fr_count_i = 0;  // Frame count in interval
    float fr_avg = 0;      // Average frequency
    size_t fr_count_a = 0; // Count measured points for averaging

    for (int ir = 0; ir < rep; ir++)
    {
        ricom_data.assign(nxy, 0);
        stem_data.assign(nxy, 0);
        com_map_x.assign(nxy, 0);
        com_map_y.assign(nxy, 0);
        reset_limits();
        std::vector<std::future<void>> futures;
        futures.reserve(4028);

        bool b_not_first = false;
        for (int iy = 0; iy < ny; iy++)
        {
            for (int ix = 0; ix < nx; ix++)
            {
                read_data<T>(data, b_not_first);
                b_not_first = true;
                // com_icom<T>(data, ix, iy, &dose_sum, &com_xy_sum);
                futures.push_back(std::async(&Ricom::com_icom<T>, this, data, ix, iy, &dose_sum, &com_xy_sum));
                fr_count_i++;

                auto mil_secs = chc::duration_cast<RICOM::double_ms>(chc::high_resolution_clock::now() - start_perf).count();
                if (mil_secs > 500.0 || fr_count == fr_total) // ~2Hz for display
                {
                    rescales_recomputes(futures);
                    if (rc_quit)
                    {
                        return;
                    };
                    auto fut_cbed = std::async(&Ricom::plot_cbed<T>, this, data);
                    fr = fr_count_i / mil_secs;
                    fr_avg += fr;
                    fr_count_a++;
                    start_perf = chc::high_resolution_clock::now();
                    fr_freq = fr_avg / fr_count_a;
                    bar.Progressed(fr_count, fr_avg / fr_count_a);
                    for (int i = 0; i < 2; i++)
                    {
                        com_public[i] = com_xy_sum[i] / fr_count_i;
                        com_xy_sum[i] = 0;
                    }
                    fr_count_i = 0;
                }
            }
            skip_frames(skip_row, data);
        }
        skip_frames(skip_img, data);

        get_futures(futures);
        bar.Progressed(fr_count, fr_avg / fr_count_a);
        
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
    size_t im_size = (nx + kernel.kernel_size * 2) * (ny + kernel.kernel_size * 2);

    // Initialize Progress bar
    ProgressBar bar(fr_total, "kHz", !b_print2file);

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
            bar.Progressed(fr_count, fr_avg / fr_count_a);
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
    img_px = nxy + (ny * skip_row) + skip_img;
    fr_total = img_px * rep;
    fr_count = 0;
    
    init_surface(nx, ny);

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
    read_head();
}

void Ricom::reset()
{
    rc_quit = false;
    fr_freq = 0;
    b_binary = false;
    b_raw = false;
    reset_limits();
}