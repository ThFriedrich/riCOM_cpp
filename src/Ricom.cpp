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
    cbed_log.assign(n_cam * n_cam, 0.0);
    srf_cbed = SDL_CreateRGBSurface(0, n_cam, n_cam, 32, 0, 0, 0, 0);
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
                 e_mag_max(-FLT_MAX), e_mag_min(FLT_MAX),
                 ricom_max(-FLT_MAX), ricom_min(FLT_MAX),
                 cbed_log(),
                 ricom_mutex(), stem_mutex(), counter_mutex(), e_field_mutex(),
                 socket(), file_path(""),
                 camera(),
                 mode(0),
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
                 comx_data(), comy_data(),
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


// Rescales the images according to updated min and max values
// and recomputes the Kernel if settings changed
inline void Ricom::rescales_recomputes()
{
    if (b_recompute_detector)
    {
        detector.compute_detector(n_cam, n_cam, offset);
        b_recompute_detector = false;
    }
    if (b_recompute_kernel)
    {
        kernel.compute_kernel();
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
}

void Ricom::process_data()
{
    // Start Thread Pool
    BoundedThreadPool pool;
    if (n_threads > 1)
        pool.init(n_threads, queue_size);

    frame_id_plot_cbed[0] = 0;
    frame_id_plot_cbed[2] = 0;

    size_t img_num = 0;
    size_t first_frame = img_num * nxy;
    size_t end_frame = (img_num + 1) * nxy;
    size_t fr_total_u = (size_t)fr_total;

    ProgressMonitor prog_mon(fr_total, !b_print2file, redraw_interval);
    p_prog_mon = &prog_mon;

    reinit_vectors_limits();

    while (processor_line != -1)
    {
        line_processor(
            img_num, 
            first_frame, 
            end_frame, 
            p_prog_mon, 
            fr_total_u, 
            &pool
            );
    }
    p_prog_mon = nullptr;
}

void Ricom::line_processor(
    size_t &img_num,
    size_t &first_frame,
    size_t &end_frame,
    ProgressMonitor *prog_mon,
    size_t &fr_total_u,
    BoundedThreadPool *pool
)
{
    int idxx = 0;
    // process newly finished lines, if there are any
    if ((int)(prog_mon->fr_count / nx) < preprocessor_line)
    {
        processor_line = (int)(prog_mon->fr_count) / nx;
        idxx = (int)(prog_mon->fr_count) % nxy;
        *prog_mon += nx;

        std::array<float, 2> com_xy = {0.0, 0.0};
        std::array<float, 2> com_xy_sum = {0.0, 0.0};
        for (size_t i = 0; i < (size_t)nx; i++)
        {
            int idxx_p_i = idxx + i;
            if ((idxx_p_i >= 0) | (nx > 1))
            {
                if (dose_data[idxx_p_i] == 0)
                {
                    comx_data[idxx_p_i] = offset[0];
                    comy_data[idxx_p_i] = offset[1];
                }
                else
                {
                    comx_data[idxx_p_i] = sumx_data[idxx_p_i] / dose_data[idxx_p_i];
                    comy_data[idxx_p_i] = sumy_data[idxx_p_i] / dose_data[idxx_p_i];
                }
                com_xy_sum[0] += comx_data[idxx_p_i];
                com_xy_sum[1] += comy_data[idxx_p_i];
                if (!b_cumulative) {
                    sumx_data[idxx_p_i] = 0;
                    sumy_data[idxx_p_i] = 0;
                    dose_data[idxx_p_i] = 0;
                }
            }
            if (b_e_mag)
            {
                com_xy[0] = comx_data[idxx_p_i];
                com_xy[1] = comy_data[idxx_p_i];
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

            fr_freq = prog_mon->fr_freq;
            rescales_recomputes();
            for (int i = 0; i < 2; i++)
            {
                com_public[i] = com_xy_sum[i] / nx;
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
        pool->wait_for_completion();
        p_prog_mon = nullptr;
        b_cumulative = false;
        b_continuous = false;
        processor_line = -1;
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
        i_line < nx;
        i_line++, idr_delay++)
        { ricom_data[idr_delay]=0; }


        for (int iy = -kernel.kernel_size; iy <= kernel.kernel_size; iy++)
        {
            for (int i_line = 0, idr_delay = idxx - kernel.kernel_size * nx;
                i_line < nx;
                i_line++, idr_delay++)
            {
                idc = idr_delay + iy * nx;
                idr_x = idr_delay % nx;
                idk = (kernel.kernel_size + iy) * kernel.k_width_sym;
                for (int ix = -kernel.kernel_size; ix <= kernel.kernel_size; ix++)
                {
                    if (((idr_x + ix) >= 0) & ((idr_x + ix) < nx))
                    {
                        // ricom_data[idr] += ((comx_data[idc + ix] - offset[0]) * -kernel.kernel_x[idk] +
                        //                     (comy_data[idc + ix] - offset[1]) * -kernel.kernel_y[idk]);

                        ricom_data[idr_delay] += ((comx_data[idc + ix] - offset[0]) * -kernel.kernel_x[idk] +
                                                  (comy_data[idc + ix] - offset[1]) * -kernel.kernel_y[idk]);
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
void Ricom::run(int mode)
{
    this->mode = mode;
    reset();
    b_busy = true;
    // Initializations
    nxy = nx * ny;
    fr_total = nxy * rep;
    fr_count = 0;
    init_surface();

    // Imaging tools
    kernel.compute_kernel();
    detector.compute_detector(n_cam, n_cam, offset);

    // Allocate memory for image arrays
    offset[0] = n_cam / 2;
    offset[1] = n_cam / 2;
    stem_data.assign(nxy, 0);
    ricom_data.assign(nxy, 0);
    comx_data.assign(nxy, 0);
    comy_data.assign(nxy, 0);
    dose_data.assign(nxy, 0);
    sumx_data.assign(nxy, 0);
    sumy_data.assign(nxy, 0);
    frame.assign(n_cam * n_cam, 0);
    airpi_data.assign(nxy, 0); // not correct, airpi_data size will be larger due to super resolution

    // Data Processing Progress
    processor_line = 0;
    preprocessor_line = 0;

    // Run camera dependent pipeline
    switch (camera)
    {
        // Merlin not implemented
        case RICOM::ADVAPIX:
        {   
            using namespace ADVAPIX_ADDITIONAL;
            ADVAPIX cam(
                nx, 
                ny, 
                n_cam, 
                dt,
                b_vSTEM,
                b_ricom,
                b_e_mag,
                b_airpi,
                &detector.radius2,
                &offset,
                &stem_data,
                &ricom_data,
                &comx_data,
                &comy_data,
                &dose_data,
                &sumx_data,
                &sumy_data,
                &frame,
                &airpi_data,
                &processor_line,
                &preprocessor_line,
                mode,
                file_path,
                &socket
            );
            cam.run();
            process_data();
            cam.terminate();
            break;
        }
        case RICOM::CHEETAH:
        {
            using namespace CHEETAH_ADDITIONAL;
            CHEETAH cam(
                nx, 
                ny, 
                n_cam, 
                dt,
                b_vSTEM,
                b_ricom,
                b_e_mag,
                b_airpi,
                &detector.radius2,
                &offset,
                &stem_data,
                &ricom_data,
                &comx_data,
                &comy_data,
                &dose_data,
                &sumx_data,
                &sumy_data,
                &frame,
                &airpi_data,
                &processor_line,
                &preprocessor_line,
                mode,
                file_path,
                &socket
            );
            cam.run();
            process_data();
            cam.terminate();
            break;
        }
    }


    // std::this_thread::sleep_for(std::chrono::milliseconds(10*1000));


    // close the reading and preprocess threads ??

    b_busy = false;
    std::cout << std::endl
              << "Reconstruction finished successfully." << std::endl;
}

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
    std::fill(comx_data.begin(), comx_data.end(), 0);
    std::fill(comy_data.begin(), comy_data.end(), 0);
    last_y = 0;
    reset_limits();
}

void Ricom::reset()
{
    rc_quit = false;
    fr_freq = 0;
    reinit_vectors_limits();
}
