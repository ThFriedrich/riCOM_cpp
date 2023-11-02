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

#include "Ricom.h"

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

    for (int iy = 0; iy < k_width_sym; iy++)
    {
        int iy_e = (iy + 1) * k_width_sym - 1;
        for (int ix = 0; ix < k_width_sym; ix++)
        {
            int ix_s = ix - kernel_size;
            int iy_s = iy - kernel_size;
            float d = ix_s * ix_s + iy_s * iy_s;
            int ix_e = k_area - iy_e + ix - 1;

            if (d > 0)
            {
                float ix_sd = (ix_s / d);
                float iy_sd = (iy_s / d);
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
    kernel_filter.assign(k_area, 0);
    float lb = pow(kernel_filter_frequency[0], 2);
    float ub = pow(kernel_filter_frequency[1], 2);

    for (int iy = 0; iy < k_width_sym; iy++)
    {
        for (int ix = 0; ix < k_width_sym; ix++)
        {
            float dist = pow(ix - kernel_size, 2) + pow(iy - kernel_size, 2);
            int ic = iy * k_width_sym + ix;
            if (dist <= ub && dist > lb)
            {
                kernel_filter[ic] = 1;
            }
        }
    }
}

// Applies the filter to the kernel
void Ricom_kernel::include_filter()
{
    FFT2D fft2d(k_width_sym, k_width_sym);
    std::vector<std::complex<float>> x2c = FFT2D::r2c(kernel_x);
    std::vector<std::complex<float>> y2c = FFT2D::r2c(kernel_y);
    fft2d.fft(x2c, x2c);
    fft2d.fft(y2c, y2c);
    for (int id = 0; id < k_area; id++)
    {
        if (kernel_filter[id] == 0.0f)
        {
            x2c[id] = {0, 0};
            y2c[id] = {0, 0};
        }
    }
    fft2d.ifft(x2c, x2c);
    fft2d.ifft(y2c, y2c);
    for (int id = 0; id < k_area; id++)
    {
        kernel_x[id] = x2c[id].real();
        kernel_y[id] = y2c[id].real();
    }
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
    std::for_each(f_approx.begin(), f_approx.end(), [f_max](float &x)
                  { x /= f_max; });
}

void Ricom_kernel::draw_surfaces()
{
    // Creating SDL Surface (holding the ricom image in CPU memory)
    srf_kx = SDL_CreateRGBSurface(0, k_width_sym, k_width_sym, 32, 0, 0, 0, 0);
    if (srf_kx == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    srf_ky = SDL_CreateRGBSurface(0, k_width_sym, k_width_sym, 32, 0, 0, 0, 0);
    if (srf_ky == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    // determine location index of value in memory
    std::pair kx_min_max = std::minmax_element(kernel_x.begin(), kernel_x.end());
    std::pair ky_min_max = std::minmax_element(kernel_y.begin(), kernel_y.end());
    // Update pixel at location

    for (int y = 0; y < k_width_sym; y++)
    {
        int ic = y * k_width_sym;
        for (int x = 0; x < k_width_sym; x++)
        {
            float valx = (kernel_x[ic + x] - kx_min_max.first[0]) / (kx_min_max.second[0] - kx_min_max.first[0]);
            SDL_Utils::draw_pixel(srf_kx, x, y, valx, 5);
            float valy = (kernel_y[ic + x] - ky_min_max.first[0]) / (ky_min_max.second[0] - ky_min_max.first[0]);
            SDL_Utils::draw_pixel(srf_ky, x, y, valy, 5);
        }
    }
}
// Computer detector distance map relative to a given centre (offset) for vSTEM
void Ricom_detector::compute_detector(int nx_cam, int ny_cam, std::array<float, 2> &offset)
{

    radius2[0] = pow(radius[0], 2);
    radius2[1] = pow(radius[1], 2);
    float d2;
    id_list.clear();
    id_list.reserve(nx_cam * ny_cam);

    for (int iy = 0; iy < ny_cam; iy++)
    {
        for (int ix = 0; ix < nx_cam; ix++)
        {
            d2 = pow((float)ix - offset[0], 2) + pow((float)iy - offset[1], 2);
            if (d2 > radius2[0] && d2 <= radius2[1])
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
    // SDL surface for STEM image
    srf_e_mag = SDL_CreateRGBSurface(0, nx, ny, 32, 0, 0, 0, 0);
    if (srf_e_mag == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    // SDL surface for CBED image
    cbed_log.assign(nx_cam * ny_cam, 0.0);
    srf_cbed = SDL_CreateRGBSurface(0, nx_cam, ny_cam, 32, 0, 0, 0, 0);
    if (srf_cbed == NULL)
    {
        std::cout << "Surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
}

////////////////////////////////////////////////
//     RICOM class method implementations     //
////////////////////////////////////////////////
Ricom::Ricom() : stem_max(-FLT_MAX), stem_min(FLT_MAX),
                 update_list(),
                 e_mag_max(-FLT_MAX), e_mag_min(FLT_MAX),
                 ricom_max(-FLT_MAX), ricom_min(FLT_MAX),
                 cbed_log(),
                 ricom_mutex(), stem_mutex(), counter_mutex(), e_field_mutex(),
                 socket(), file_path(""),
                 camera(),
                 mode(RICOM::FILE),
                 b_print2file(false),
                 redraw_interval(50),
                 last_y(0),
                 p_prog_mon(nullptr),
                 b_busy(false),
                 update_offset(true),
                 b_vSTEM(false), b_e_mag(false),
                 b_plot_cbed(true),
                 b_plot2SDL(false), b_recompute_detector(false),
                 b_recompute_kernel(false), detector(),
                 kernel(),
                 offset{128, 128}, com_public{0.0, 0.0},
                 com_map_x(), com_map_y(),
                 ricom_data(),
                 stem_data(),
                 nx(1024), ny(1024), nxy(0),
                 rep(1), fr_total(0),
                 skip_row(1), skip_img(0),
                 n_threads(1), queue_size(64),
                 fr_freq(0.0), fr_count(0.0), fr_count_total(0.0),
                 rescale_ricom(false), rescale_stem(false),
                 rc_quit(false),
                 srf_ricom(NULL), ricom_cmap(9),
                 srf_stem(NULL), stem_cmap(9),
                 srf_cbed(NULL), cbed_cmap(5),
                 srf_e_mag(NULL), e_mag_cmap(12)
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
void Ricom::com(std::vector<T> *data, std::array<float, 2> &com)
{
    float dose = 0;
    std::vector<size_t> sum_x(nx_cam);
    std::vector<size_t> sum_y(ny_cam);
    sum_x.assign(nx_cam, 0);
    sum_y.assign(ny_cam, 0);
    com = {0.0, 0.0};

    for (int idy = 0; idy < ny_cam; idy++)
    {
        size_t y_nx = idy * nx_cam;
        size_t sum_x_temp = 0;
        for (int idx = 0; idx < nx_cam; idx++)
        {
            T px = data->data()[y_nx + idx];
            swap_endianess(px);
            sum_x_temp += px;
            sum_y[idx] += px;
        }
        sum_x[idy] = sum_x_temp;
        dose += sum_x_temp;
    }

    if (dose > 0)
    {
        for (int i = 0; i < nx_cam; i++)
        {
            com[0] += sum_x[i] * camera.v[i];
        }
        for (int i = 0; i < ny_cam; i++)
        {
            com[1] += sum_y[i] * camera.u[i];
        }
        for (int i = 0; i < 2; i++)
        {
            com[i] = (com[i] / dose);
        }
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
    if (y0 < (kernel.kernel_size+2)) {
        ricom_min = 0;
        ricom_max = 0;
    }

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
    if (ricom_data[idr] < ricom_min) {ricom_min = ricom_data[idr];}
    if (ricom_data[idr] > ricom_max) {ricom_max = ricom_data[idr];}

    float val = (ricom_data[idr] - ricom_min) / ((ricom_max - ricom_min));

    // Update pixel at location
    SDL_Utils::draw_pixel(srf_ricom, idx, idy, val, ricom_cmap);
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
    if (y0==0) {
        stem_min = stem_data[y0*nx];
        stem_max = stem_data[y0*nx];
    }

    std::lock_guard<std::mutex> lock(stem_mutex);
    for (int y = y0; y <= ye; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_stem_pixel(x, y);
        }
    }
}

void Ricom::draw_e_field_image(int y0, int ye)
{
    std::lock_guard<std::mutex> lock(e_field_mutex);
    int idr;
    for (int y = y0; y <= ye; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_e_field_pixel(x, y);
            idr = y * nx + x;
        }
    }
}

void Ricom::draw_e_field_image()
{
    std::lock_guard<std::mutex> lock(e_field_mutex);
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            set_e_field_pixel(x, y);
        }
    }
}

// set pixel in stem image srf_stem at location idx, idy to value corresponding
// value in stem_data array
void Ricom::set_stem_pixel(size_t idx, size_t idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;

    if (stem_data[idr] < stem_min) {stem_min = stem_data[idr];}
    if (stem_data[idr] > stem_max) {stem_max = stem_data[idr];}

    float val = (stem_data[idr] - stem_min) / (stem_max - stem_min);

    // Update pixel at location
    SDL_Utils::draw_pixel(srf_stem, idx, idy, val, stem_cmap);
}

// set pixel in stem image srf_stem at location idx, idy to value corresponding
// value in stem_data array
void Ricom::set_e_field_pixel(size_t idx, size_t idy)
{
    // determine location index of value in memory
    int idr = idy * nx + idx;
    float mag = (abs(e_field_data[idr]) - e_mag_min) / (e_mag_max - e_mag_min);
    float ang = arg(e_field_data[idr]);

    // Update pixel at location
    SDL_Utils::draw_pixel(srf_e_mag, idx, idy, ang, mag, e_mag_cmap);
}

// Draw a CBED in log-scale to the SDL surface srf_cbed
template <typename T>
void Ricom::plot_cbed(std::vector<T> &cbed_data)
{
    float v_min = INFINITY;
    float v_max = 0.0;

    for (size_t id = 0; id < cbed_data.size(); id++)
    {
        T vl = cbed_data[id];
        swap_endianess(vl);
        float vl_f = log1p((float)vl);
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

    float v_rng = v_max - v_min;
    for (int ix = 0; ix < ny_cam; ix++)
    {
        int iy_t = camera.v[ix] * nx_cam;
        for (int iy = 0; iy < nx_cam; iy++)
        {
            float vl_f = cbed_log[iy_t + camera.u[iy]];
            float val = (vl_f - v_min) / v_rng;
            SDL_Utils::draw_pixel(srf_cbed, ix, iy, val, cbed_cmap);
        }
    }
}

// Rescales the images according to updated min and max values
// and recomputes the Kernel if settings changed
inline void Ricom::rescales_recomputes()
{
    if (b_recompute_detector)
    {
        detector.compute_detector(nx_cam, ny_cam, offset);
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

    if (b_e_mag)
    {
        if (rescale_e_mag)
        {
            rescale_e_mag = false;
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
void Ricom::com_icom(std::vector<T> data, int ix, int iy, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon)
{
    std::vector<T> *data_ptr = &data;

    std::array<float, 2> com_xy = {0.0, 0.0};
    com<T>(data_ptr, com_xy);
    icom(com_xy, ix, iy);

    size_t id = iy * nx + ix;

    if (b_vSTEM)
    {
        stem(data_ptr, id);
    }
    if (b_e_mag)
    {
        compute_electric_field(com_xy, id);
    }

    com_xy_sum->at(0) += com_xy[0];
    com_xy_sum->at(1) += com_xy[1];

    com_map_x[id] = com_xy[0];
    com_map_y[id] = com_xy[1];

    counter_mutex.lock();
    ++(*p_prog_mon);
    fr_count = p_prog_mon->fr_count;
    if (p_prog_mon->report_set)
    {
        update_surfaces(iy, *data_ptr);
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
void Ricom::com_icom(std::vector<T> *data_ptr, int ix, int iy, std::array<float, 2> *com_xy_sum, ProgressMonitor *p_prog_mon)
{
    std::array<float, 2> com_xy = {0.0, 0.0};
    com<T>(data_ptr, com_xy);
    icom(com_xy, ix, iy);

    size_t id = iy * nx + ix;
    if (b_vSTEM)
    {
        stem(data_ptr, id);
    }
    if (b_e_mag)
    {
        compute_electric_field(com_xy, id);
    }
    com_xy_sum->at(0) += com_xy[0];
    com_xy_sum->at(1) += com_xy[1];

    ++(*p_prog_mon);
    com_map_x[id] = com_xy[0];
    com_map_y[id] = com_xy[1];
    fr_count = p_prog_mon->fr_count;
    if (p_prog_mon->report_set)
    {
        update_surfaces(iy, *data_ptr);
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

// Compute electric field magnitude
void Ricom::compute_electric_field(std::array<float, 2> &com_xy, size_t id)
{
    float e_mag = std::hypot(com_xy[0] - offset[0], com_xy[1] - offset[1]);
    float e_ang = atan2(com_xy[0] - offset[0], com_xy[1] - offset[1]);
    if (e_mag > e_mag_max)
    {
        e_mag_max = e_mag;
        rescale_e_mag = true;
    }
    if (e_mag < e_mag_min)
    {
        e_mag_min = e_mag;
        rescale_e_mag = true;
    }
    e_field_data[id] = std::polar(e_mag, e_ang);
}

// Process FRAME_BASED camera data
template <typename T, class CameraInterface>
void Ricom::process_data(CAMERA::Camera<CameraInterface, CAMERA::FRAME_BASED> *camera_spec)
{
    // Memory allocation
    int cam_xy = camera_spec->nx_cam * camera_spec->ny_cam;
    std::vector<T> data(cam_xy);
    std::vector<T> *p_data = &data;
    BoundedThreadPool pool;

    // Start Thread Pool
    if (n_threads > 1)
        pool.init(n_threads, queue_size);

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
                                   { com_icom<T>(data, ix, iy, p_com_xy_sum, p_prog_mon); });
                }
                else
                {
                    com_icom<T>(p_data, ix, iy, p_com_xy_sum, p_prog_mon);
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

        if (update_offset)
        {
            offset[0] = com_public[0];
            offset[1] = com_public[1];
        }
    }
    p_prog_mon = nullptr;
}

template <typename T>
void Ricom::update_surfaces(int iy, std::vector<T> &frame)
{
    if (b_plot2SDL)
    {
        // draw_ricom_image((std::max)(0, last_y - kernel.kernel_size), (std::min)(iy + kernel.kernel_size, ny - 1));
        draw_ricom_image((std::max)(0, last_y % ny - kernel.kernel_size - 1), iy);
        if (b_vSTEM)
        {
            draw_stem_image(iy, iy+1);
        }
        if (b_e_mag)
        {
            draw_e_field_image(iy, iy+1);
        }
    }
    if (b_plot_cbed)
    {
        plot_cbed(frame);
    }
}

template <class CameraInterface>
void Ricom::process_data(CAMERA::Camera<CameraInterface, CAMERA::EVENT_BASED> *camera_spec)
{
    // Memory allocation
    std::vector<size_t> dose_map(nxy);
    std::vector<size_t> sumx_map(nxy);
    std::vector<size_t> sumy_map(nxy);
    std::vector<size_t> frame(camera_spec->nx_cam * camera_spec->ny_cam);

    BoundedThreadPool pool;

    camera_spec->scan_x = nx;
    camera_spec->scan_y = ny;
    camera.group_size = nx;

    // Start Thread Pool
    if (n_threads > 1)
        pool.init(n_threads, queue_size);

    frame_id_plot_cbed[0] = 0;
    frame_id_plot_cbed[2] = 0;


    size_t img_num = 0;
    size_t first_frame = img_num * nxy;
    size_t end_frame = (img_num + 1) * nxy;
    size_t fr_total_u = (size_t)fr_total;
    int processor_line = 0;
    int preprocessor_line = 0;

    ProgressMonitor prog_mon(fr_total, !b_print2file, redraw_interval);
    p_prog_mon = &prog_mon;

    dose_map.assign(nxy, 0);
    sumx_map.assign(nxy, 0);
    sumy_map.assign(nxy, 0);
    reinit_vectors_limits();

    std::clock_t start_time = std::clock();
    bool fin = false;

    // start the reading and preprocess threads
    camera_spec->read_frame_com_cbed(
        dose_map, sumx_map, sumy_map,
        stem_data, b_vSTEM,
        offset, detector.radius2,
        frame, frame_id_plot_cbed,
        processor_line, preprocessor_line, first_frame, end_frame
    );

    if (preprocessor_line < 1)
    {
        start_time = std::clock();
    }
    while (!fin)
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // std::cout << preprocessor_line << std::endl;

        line_processor(
            img_num, dose_map, sumx_map, sumy_map, frame,
            first_frame, end_frame, p_prog_mon, camera_spec, fr_total_u, fin, &pool,
            processor_line, preprocessor_line);
        // std::cout<<preprocessor_line<<std::endl;
    }
    std::clock_t end_time = std::clock();

    // close the reading and preprocess threads
    camera_spec->read_frame_com_cbed(
        dose_map, sumx_map, sumy_map,
        stem_data, b_vSTEM,
        offset, detector.radius2,
        frame, frame_id_plot_cbed,
        processor_line, preprocessor_line, first_frame, end_frame
    );

    double time_taken = static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;
    int cnt = 0;
    for (int i = 0; i < nxy; i++)
    {
        cnt += dose_map[i];
    }
    std::cout << cnt << " electrons in " << time_taken << " seconds" << std::endl;
    p_prog_mon = nullptr;
}

template <class CameraInterface>
void Ricom::line_processor(
    size_t &img_num,
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map,
    std::vector<size_t> &sumy_map,
    std::vector<size_t> &frame,
    size_t &first_frame,
    size_t &end_frame,
    ProgressMonitor *prog_mon,
    CAMERA::Camera<CameraInterface, CAMERA::EVENT_BASED> *camera_spec,
    size_t &fr_total_u,
    bool &fin,
    BoundedThreadPool *pool,
    int &processor_line,
    int &preprocessor_line
)
{
    int delay_update = (kernel.kernel_size+1)*2*nx;
    int idxx = 0;
    // process newly finished lines, if there are any
    if ((int)(prog_mon->fr_count / nx) < preprocessor_line)
    {
        processor_line = (int)(prog_mon->fr_count) / nx;
        idxx = (int)(prog_mon->fr_count) % nxy;
        *prog_mon += nx;

        // if (idxx==delay_update){
        //     reset_limits();
        // }

        // actual processing start
        std::array<float, 2> com_xy = {0.0, 0.0};
        std::array<float, 2> com_xy_sum = {0.0, 0.0};
        for (size_t i = 0; i < camera.group_size; i++)
        {
            int idxx_p_i = idxx + i;
            if ((idxx_p_i >= 0) | (camera.group_size > 1))
            {
                if (dose_map[idxx_p_i] == 0)
                {
                    com_map_x[idxx_p_i] = offset[0];
                    com_map_y[idxx_p_i] = offset[1];
                }
                else
                {
                    com_map_x[idxx_p_i] = sumx_map[idxx_p_i] / dose_map[idxx_p_i];
                    com_map_y[idxx_p_i] = sumy_map[idxx_p_i] / dose_map[idxx_p_i];
                }
                com_xy_sum[0] += com_map_x[idxx_p_i];
                com_xy_sum[1] += com_map_y[idxx_p_i];
                if (!b_cumulative) {
                    sumx_map[idxx_p_i] = 0;
                    sumy_map[idxx_p_i] = 0;
                    dose_map[idxx_p_i] = 0;
                }
            }
            if (b_e_mag)
            {
                com_xy[0] = com_map_x[idxx_p_i];
                com_xy[1] = com_map_y[idxx_p_i];
                compute_electric_field(com_xy, idxx_p_i);
            }
        }

        if (n_threads > 1)
        {
            pool->push_task([=]
                            { icom_group_classical(idxx); });
        }
        else
        {
            icom_group_classical(idxx);
        }
        // end of line handler
        int update_line = idxx / nx - kernel.kernel_size*2;
        if ((prog_mon->report_set) && (update_line)>0)
        {
            update_surfaces(update_line, frame);

            if (b_plot_cbed)
            {
                frame_id_plot_cbed[2] = 1;
            }
            fr_freq = prog_mon->fr_freq;
            rescales_recomputes();
            for (int i = 0; i < 2; i++)
            {
                com_public[i] = com_xy_sum[i] / camera.group_size;
                com_xy_sum[i] = 0;
            }
            prog_mon->reset_flags();
        }
    }

    // end of image handler

    if (prog_mon->fr_count >= end_frame)

    {
        if (b_continuous) {
            prog_mon->fr_total += nxy;
            fr_total_u += nxy;
        }
        if (prog_mon->fr_count != fr_total_u)
        {
            img_num++;
            first_frame = img_num * nxy;
            end_frame = (img_num + 1) * nxy;
            if (!b_cumulative)
            {
                // reinit_vectors_limits();
                //dose_map.assign(nxy, 0);
                //sumx_map.assign(nxy, 0);
                //sumy_map.assign(nxy, 0);
                // std::fill(stem_data.begin(), stem_data.end(), 0);
            }
        }
        if (update_offset)
        {
            offset[0] = com_public[0];
            offset[1] = com_public[1];
        }
    }

    // end of recon handler
    if (((prog_mon->fr_count >= fr_total_u) && (!b_continuous)) || rc_quit)
    {
        // camera_spec->finish = true;
        pool->wait_for_completion();
        p_prog_mon = nullptr;
        fin = true;
        b_cumulative = false;
        b_continuous = false;
        processor_line = -1;
    }
}

void Ricom::icom_group_decompose(int idxx)
{
    id_x_y idr;
    for (int idc = idxx; idc < (idxx + (int)camera.group_size); idc++)
    {
        for (int id = 0; id < kernel.k_area; id++)
        {
            update_list.shift(idr, id, idc);
            if (idr.valid)
            {
                ricom_data[idr.id] += ((com_map_x[idc] - offset[0]) * kernel.kernel_x[id] +
                                       (com_map_y[idc] - offset[1]) * kernel.kernel_y[id]);
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
}

void Ricom::icom_group_classical(int idxx)
{
    int idk;
    int idc;
    int id;
    int idr_x;
    int idr_delay;
    int k_bias = kernel.k_width_sym * (kernel.kernel_size + 1);

    if (((idxx / nx - 2*kernel.kernel_size) >= 0))
    {


        for (int i_line = 0, idr_delay = idxx - kernel.kernel_size * nx;
        i_line < (int)camera.group_size;
        i_line++, idr_delay++)
        { ricom_data[idr_delay]=0; }


        for (int iy = -kernel.kernel_size; iy <= kernel.kernel_size; iy++)
        {
            for (int i_line = 0, idr_delay = idxx - kernel.kernel_size * nx;
                i_line < (int)camera.group_size;
                i_line++, idr_delay++)
            {
                idc = idr_delay + iy * nx;
                idr_x = idr_delay % nx;
                idk = (kernel.kernel_size + iy) * kernel.k_width_sym;
                for (int ix = -kernel.kernel_size; ix <= kernel.kernel_size; ix++)
                {
                    if (((idr_x + ix) >= 0) & ((idr_x + ix) < nx))
                    {
                        // ricom_data[idr] += ((com_map_x[idc + ix] - offset[0]) * -kernel.kernel_x[idk] +
                        //                     (com_map_y[idc + ix] - offset[1]) * -kernel.kernel_y[idk]);

                        ricom_data[idr_delay] += ((com_map_x[idc + ix] - offset[0]) * -kernel.kernel_x[idk] +
                                                  (com_map_y[idc + ix] - offset[1]) * -kernel.kernel_y[idk]);
                    }
                    ++idk;
                }
            }
        }
        last_y = (idxx / nx - kernel.kernel_size);
        fr_count = idxx;
    }

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
        detector.compute_detector(nx_cam, ny_cam, offset);
    }

    // Compute the integration Kenel
    kernel.compute_kernel();

    // Allocate the ricom image and COM arrays
    ricom_data.resize(nxy);
    update_list.init(kernel, nx, ny);
    com_map_x.resize(nxy);
    com_map_y.resize(nxy);
    stem_data.resize(nxy);
    e_field_data.resize(nxy);

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
template void Ricom::run_reconstruction<CheetahInterface>(RICOM::modes);
template void Ricom::process_data<uint8_t>(CAMERA::Camera<MerlinInterface, CAMERA::FRAME_BASED> *camera_spec);
template void Ricom::process_data<uint16_t>(CAMERA::Camera<MerlinInterface, CAMERA::FRAME_BASED> *camera_spec);
template void Ricom::process_data<uint32_t>(CAMERA::Camera<MerlinInterface, CAMERA::FRAME_BASED> *camera_spec);
template void Ricom::process_data(CAMERA::Camera<TimepixInterface, CAMERA::EVENT_BASED> *camera_spec);
template void Ricom::process_data(CAMERA::Camera<CheetahInterface, CAMERA::EVENT_BASED> *camera_spec);

// Helper functions
void Ricom::reset_limits()
{
    // ricom_max = -FLT_MAX;
    // ricom_min = FLT_MAX;
    ricom_max = 0;
    ricom_min = 0;
    stem_max = -FLT_MAX;
    stem_min = FLT_MAX;
}

void Ricom::reinit_vectors_limits()
{
    std::fill(ricom_data.begin(), ricom_data.end(), 0);
    std::fill(stem_data.begin(), stem_data.end(), 0);
    std::fill(com_map_x.begin(), com_map_x.end(), 0);
    std::fill(com_map_y.begin(), com_map_y.end(), 0);
    last_y = 0;
    reset_limits();
}

void Ricom::reset()
{
    rc_quit = false;
    fr_freq = 0;
    reinit_vectors_limits();
}

enum CAMERA::Camera_model Ricom::select_mode_by_file(const char *filename)
{
    file_path = filename;
    if (std::filesystem::path(filename).extension() == ".t3p")
    {
        mode = RICOM::FILE;
        return CAMERA::TIMEPIX;
    }
    else if (std::filesystem::path(filename).extension() == ".mib")
    {
        mode = RICOM::FILE;
        return CAMERA::MERLIN;
    }
    else if (std::filesystem::path(filename).extension() == ".tpx3")
    {
        mode = RICOM::FILE;
        return CAMERA::CHEETAH;
    }
    else
    {
        return CAMERA::TIMEPIX;
    }
}

void RICOM::run_ricom(Ricom *r, RICOM::modes mode)
{
    switch (r->camera.model)
    {
    case CAMERA::MERLIN:
        r->run_reconstruction<MerlinInterface>(mode);
        break;
    case CAMERA::TIMEPIX:
        r->run_reconstruction<TimepixInterface>(mode);
        break;
    case CAMERA::CHEETAH:
        r->run_reconstruction<CheetahInterface>(mode);
        break;
    default:
        break;
    }
}

void RICOM::run_connection_script(Ricom *ricom, MerlinSettings *merlin, const std::string &python_path)
{

    int m_fr_total = ((ricom->nx + ricom->skip_row) * ricom->ny + ricom->skip_img) * ricom->rep;

    std::ofstream run_script("run_script.py");
    run_script << "from merlin_interface.merlin_interface import MerlinInterface" << std::endl;
    run_script << "m = MerlinInterface(tcp_ip = \"" << ricom->socket.ip << "\" , tcp_port=" << merlin->com_port << ")" << std::endl;
    run_script << "m.hvbias = " << merlin->hvbias << std::endl;
    run_script << "m.threshold0 = " << merlin->threshold0 << std::endl;
    run_script << "m.threshold1 = " << merlin->threshold1 << std::endl;
    run_script << "m.continuousrw = " << (int)merlin->continuousrw << std::endl;
    run_script << "m.counterdepth = " << ricom->camera.depth << std::endl;
    run_script << "m.acquisitiontime = " << merlin->dwell_time << std::endl;
    run_script << "m.acquisitionperiod = " << merlin->dwell_time << std::endl;
    run_script << "m.numframestoacquire = " << m_fr_total + 1 << std::endl; // Ich verstehe nicht warum, aber er werkt.
    run_script << "m.fileenable = " << (int)merlin->save << std::endl;
    run_script << "m.runheadless = " << (int)merlin->headless << std::endl;
    run_script << "m.fileformat = " << (int)merlin->raw * 2 << std::endl;
    run_script << "m.triggerstart = " << merlin->trigger << std::endl;
    run_script << "m.startacquisition()";
    run_script.close();

    std::string command = python_path + " run_script.py";
    int r = std::system(command.c_str());
    if (r != 0)
    {
        std::cout << "main::run_connection_script: Cannot execute python run_script. Shell exited with code " << r << std::endl;
    }
}