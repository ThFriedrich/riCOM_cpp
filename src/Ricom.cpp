
#ifdef WIN32
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
    int kws2 = k_width_sym * k_width_sym;
    kernel_x.assign(kws2, 0);
    kernel_y.assign(kws2, 0);
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

            ix_e = kws2 - iy_e + ix - 1;

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

    for (size_t iy = 0; iy < ny_merlin; iy++)
    {
        for (size_t ix = 0; ix < nx_merlin; ix++)
        {
            d2 = pow((float)ix - offset[0], 2) + pow((float)iy - offset[1], 2);
            if (d2 > rad_in2 && d2 <= rad_out2)
            {
                id_list.push_back(iy * nx_merlin + ix);
            }
        }
    }
}

void Ricom_detector::compute_detector(std::array<float, 2> &offset)
{
    float rad_in2 = pow(radius[0], 2), rad_out2 = pow(radius[1], 2);
    float d2;
    id_list.clear();

    for (size_t iy = 0; iy < ny_merlin; iy++)
    {
        for (size_t ix = 0; ix < nx_merlin; ix++)
        {
            d2 = pow((float)ix - offset[0], 2) + pow((float)iy - offset[1], 2);
            if (d2 > rad_in2 && d2 <= rad_out2)
            {
                id_list.push_back(iy * nx_merlin + ix);
            }
        }
    }
}

////////////////////////////////////////////////
//           SDL plotting methods             //
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

void Ricom::draw_pixel(SDL_Surface *surface, int x, int y, float val)
{
    cmap::Color c = cmap::GetColor(val, cmap::ColormapType::Viridis);
    Uint32 px = SDL_MapRGB(surface->format, (Uint8)(c.b() * 255), (Uint8)(c.g() * 255), (Uint8)(c.r() * 255));
    Uint32 *const target_pixel = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
    *target_pixel = px;
}

////////////////////////////////////////////////
//     RICOM class method implementations     //
////////////////////////////////////////////////

Ricom::~Ricom(){};

void Ricom::init_uv()
{
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
void Ricom::com(std::vector<T> &data, std::array<float, 2> &com, int &dose_sum)
{
    float dose = 0;
    T px;
    size_t sum_x_temp;

    com[0] = 0;
    com[1] = 0;

    size_t y_nx;
    for (size_t idy = 0; idy < ny_merlin; idy++)
    {
        y_nx = idy * nx_merlin;
        sum_x_temp = 0;
        for (size_t idx = 0; idx < nx_merlin; idx++)
        {
            px = data[y_nx + idx];
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
            sum_x[i] = 0;
        }
        for (size_t i = 0; i < ny_merlin; i++)
        {
            com[1] += sum_y[i] * u[i];
            sum_y[i] = 0;
        }

        com[0] = (com[0] / dose);
        com[1] = (com[1] / dose);
        dose_sum += dose;
    }
}

template <typename T>
void Ricom::stem(std::vector<T> &data, size_t id_stem, bool &rescale)
{
    T px;
    size_t stem_temp = 0;
    for (size_t id : detector.id_list)
    {
        px = data[id];
        byte_swap(px);
        stem_temp += px;
    }
    if (stem_temp > stem_max)
    {
        stem_max = stem_temp;
        rescale = true;
    }
    if (stem_temp < stem_min)
    {
        stem_min = stem_temp;
        rescale = true;
    }
    stem_data[id_stem] = stem_temp;
}

void Ricom::icom(std::array<float, 2> &com, int x, int y, bool &rescale)
{
    int idk = 0;
    int idr = 0;
    int idk2 = 0;
    int idyt = 0;
    int idc = px_per_row * y + x;
    float com_x = com[0] - offset[0];
    float com_y = com[1] - offset[1];

    for (int idy = 0; idy < kernel.k_width_sym; idy++)
    {
        idyt = idy * px_per_row;
        idk = idy * kernel.k_width_sym;
        for (int idx = 0; idx < kernel.k_width_sym; idx++)
        {
            idk2 = idk + idx;
            idr = idc + idx + idyt;
            ricom_data[idr] += com_x * kernel.kernel_x[idk2] + com_y * kernel.kernel_y[idk2];
            if (idx == 0 && idy == 0 && x > kernel.kernel_size && y > kernel.kernel_size)
            {
                if (ricom_data[idr] > ricom_max)
                {
                    ricom_max = ricom_data[idr];
                    rescale = true;
                }
                if (ricom_data[idr] < ricom_min)
                {
                    ricom_min = ricom_data[idr];
                    rescale = true;
                }
            }
        }
    }
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
}

void Ricom::set_ricom_image_kernel(int ix, int iy)
{
    for (int y = (std::max)((iy - kernel.kernel_size), 0); y <= iy; y++)
    {
        for (int x = (std::max)((ix - kernel.kernel_size), 0); x <= ix; x++)
        {
            set_ricom_pixel(x, y);
        }
    }
}

void Ricom::set_ricom_pixel(size_t idx, size_t idy)
{
    // determine location index of value in memory
    int idr = (kernel.kernel_size + idy) * px_per_row + (idx + kernel.kernel_size);
    float val = (ricom_data[idr] - ricom_min) / (ricom_max - ricom_min);

    // Update pixel at location
    draw_pixel(srf_ricom, idx, idy, val);
}

void Ricom::set_stem_pixel(size_t idx, size_t idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float val = (stem_data[idr] - stem_min) / (stem_max - stem_min);

    // Update pixel at location
    draw_pixel(srf_stem, idx, idy, val);
}

template <typename T>
void Ricom::plot_cbed(std::vector<T> &data)
{
    std::vector<T> cbed_data = data;
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
            draw_pixel(srf_cbed, ix, iy, val);
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
}

template <typename T>
bool Ricom::process_frames()
{
    // Memory allocation
    std::vector<T> data(ds_merlin);
    std::array<float, 2> com_xy = {0.0, 0.0};
    std::array<float, 2> com_xy_sum = {0.0, 0.0};
    int dose_sum = 0;
    size_t im_size = (nx + kernel.kernel_size * 2) * (ny + kernel.kernel_size * 2);

    // Initialize Progress bar
    ProgressBar *bar = new ProgressBar(nxy);

    // Performance measurement
    auto start_perf = chc::high_resolution_clock::now();

    float fr = 0;          // Frequncy per frame
    size_t fr_count_i = 0; // Frame count in interval
    float fr_avg = 0;      // Average frequency
    size_t fr_count_a = 0; // Count measured points for averaging

    bool rescale = false;
    bool rescale_stem = false;
    size_t idx = 0;

    for (int ir = 0; ir < rep; ir++)
    {
        ricom_data.assign(im_size, 0);
        stem_data.assign(nxy, 0);

        for (int iy = 0; iy < ny; iy++)
        {
            idx = iy * nx;
            for (int ix = 0; ix < nx; ix++)

            {
                read_data<T>(data);
                com<T>(data, com_xy, dose_sum);
                icom(com_xy, ix, iy, rescale);
                set_ricom_image_kernel(ix, iy);
                if (use_detector)
                {
                    stem(data, idx + ix, rescale_stem);
                    set_stem_pixel(ix, iy);
                }
                com_xy_sum[0] += com_xy[0];
                com_xy_sum[1] += com_xy[1];
                fr_count_i++;
                fr_count++;
                auto mil_secs = std::chrono::duration_cast<RICOM::double_ms>(chc::high_resolution_clock::now() - start_perf).count();
                if (mil_secs > 500.0 || fr_count == nxy) // ~50Hz for display
                {
                    if (rc_quit)
                    {
                        return false;
                    };
                    if (rescale)
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
                        detector.compute_detector(offset);
                        b_recompute_detector = false;
                    }
                    if (b_recompute_kernel)
                    {
                        kernel.compute_kernel();
                        b_recompute_kernel = false;
                    }
                    plot_cbed<T>(data);
                    rescale = false;
                    rescale_stem = false;
                    fr = fr_count_i / mil_secs;
                    fr_avg += fr;
                    fr_count_a++;
                    start_perf = chc::high_resolution_clock::now();
                    fr_freq = fr_avg / fr_count_a;
                    bar->Progressed(fr_count, fr_avg / fr_count_a, "kHz");
                    com_public[0] = com_xy_sum[0] / (float)fr_count_i;
                    com_public[1] = com_xy_sum[1] / (float)fr_count_i;
                    com_xy_sum = {0.0, 0.0};
                    fr_count_i = 0;
                }

                if (!read_head())
                {
                    return false;
                }
            }
        }

        if (update_offset && dose_sum > pow(10.0, update_dose_lowbound))
        {
            offset[0] = com_public[0];
            offset[1] = com_public[1];
            dose_sum = 0;
        }
        reset_limits();
    }
    delete bar;
    return true;
}

bool Ricom::run_merlin()
{
    if (mode == RICOM::LIVE)
    {
        if (read_aquisition_header() == -1)
        {
            return false;
        }
    }

    // Run the main loop
    if (read_head())
    {
        if (dtype == "U08")
        {
            return process_frames<unsigned char>();
        }
        else if (dtype == "U16")
        {
            return process_frames<unsigned short>();
        }
        else if (dtype == "R64")
        {
            b_raw = true;
            b_binary = false;
            switch (depth)
            {
            case 1:
                b_binary = true;
                return process_frames<unsigned char>();
                break;
            case 6:
                return process_frames<unsigned char>();
                break;
            case 12:
                return process_frames<unsigned short>();
                break;
            default:
                return false;
                break;
            }
        }
    }
    else
    {
        return false;
    }
}

bool Ricom::process_timepix_stream()
{
    // Memory allocation
    std::vector<size_t> dose_map(nxy);
    std::vector<size_t> sumx_map(nxy);
    std::vector<size_t> sumy_map(nxy);
    dose_map.assign(nxy, 0);
    sumx_map.assign(nxy, 0);
    sumy_map.assign(nxy, 0);

    std::array<float, 2> com_xy = {0.0, 0.0};
    std::array<float, 2> com_xy_sum = {0.0, 0.0};
    int dose_sum = 0;
    size_t im_size = (nx + kernel.kernel_size * 2) * (ny + kernel.kernel_size * 2);

    // Initialize Progress bar
    ProgressBar *bar = new ProgressBar(nxy);

    // Performance measurement
    auto start_perf = chc::high_resolution_clock::now();

    float fr = 0;          // Frequncy per frame
    size_t fr_count_i = 0; // Frame count in interval
    float fr_avg = 0;      // Average frequency
    size_t fr_count_a = 0; // Count measured points for averaging

    bool rescale = false;
    bool rescale_stem = false;
    int idx = 0;
    int idxx = 0;

    for (int ir = 0; ir < rep; ir++)
    {
        ricom_data.assign(im_size, 0);
        stem_data.assign(nxy, 0);

        for (int iy = 0; iy < ny; iy++)
        {
            idx = iy * nx;
            for (int ix = 0; ix < nx; ix++)
            {
                idxx = idx + ix;
                read_com_ti(dose_map, sumx_map, sumy_map, nxy);
                if (dose_map[idxx] > 0)
                {
                    com_xy[0] = sumx_map[idxx] / dose_map[idxx];
                    com_xy[1] = sumy_map[idxx] / dose_map[idxx];

                    if (idxx >= 1)
                    {
                        icom(com_xy, (idxx) % nx, floor((idxx) / nx), rescale); // process one frame before the probe position to avoid toa problem
                        set_ricom_image_kernel(ix, iy);
                    }

                    com_xy_sum[0] += com_xy[0];
                    com_xy_sum[1] += com_xy[1];
                }
                if (use_detector)
                {
                    set_stem_pixel(ix, iy);
                }

                fr_count_i++;
                fr_count++;
                auto mil_secs = std::chrono::duration_cast<RICOM::double_ms>(chc::high_resolution_clock::now() - start_perf).count();
                if (mil_secs > 500.0 || fr_count == nxy) // ~50Hz for display
                {

                    if (rc_quit)
                    {
                        return false;
                    };
                    if (rescale)
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
                        detector.compute_detector(offset);
                        b_recompute_detector = false;
                    }
                    if (b_recompute_kernel)
                    {
                        kernel.compute_kernel();
                        b_recompute_kernel = false;
                    }
                    // plot_cbed<T>(data);
                    rescale = false;
                    rescale_stem = false;
                    fr = fr_count_i / mil_secs;
                    fr_avg += fr;
                    fr_count_a++;
                    start_perf = chc::high_resolution_clock::now();
                    fr_freq = fr_avg / fr_count_a;
                    com_public[0] = com_xy_sum[0] / (float)fr_count_i;
                    com_public[1] = com_xy_sum[1] / (float)fr_count_i;
                    com_xy_sum = {0.0, 0.0};
                    fr_count_i = 0;
                    bar->Progressed(fr_count, fr_avg / fr_count_a, "kHz");
                }
            }
        }
        reset_limits();
    }
    delete bar;
    return true;
}
bool Ricom::run_timepix()
{
    // Run the main loop
    return process_timepix_stream();
}

void Ricom::run()
{
    nxy = nx * ny;
    init_surface(nx, ny);

    if (use_detector)
    {
        detector.compute_detector();
        stem_data.reserve(nxy);
    }

    // Compute the integration Kenel
    kernel.compute_kernel();
    px_per_row = (kernel.k_width_sym + nx - 1);

    // Allocate the ricom image, including zero padding for kernel size
    size_t im_size = (nx + kernel.kernel_size * 2) * (ny + kernel.kernel_size * 2);
    ricom_data.reserve(im_size);

    init_uv();

    bool b_quit = true;

    switch (detector_type)
    {
    case RICOM::MERLIN:
        merlin_init(mode);
        b_quit = run_merlin();
        break;
    case RICOM::TIMEPIX:
        timepix_init(mode);
        b_quit = run_timepix();
        break;
    default:
        break;
    }

    if (b_quit)
    {
        std::cout << "Quitting..." << std::endl;
    }
    else
    {
        std::cout << "All done. Exiting..." << std::endl;
    }
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
}

void Ricom::reset_limits()
{
    fr_count = 0;
    ricom_max = -FLT_MAX;
    ricom_min = FLT_MAX;
    stem_max = -FLT_MAX;
    stem_min = FLT_MAX;
    if (mode == RICOM::modes::FILE)
    {
        mib_stream.clear();
        mib_stream.seekg(0, std::ios::beg);
        read_head();
    }
}
void Ricom::reset()
{
    rc_quit = false;
    fr_freq = 0;
    b_binary = false;
    b_raw = false;
    reset_limits();
}