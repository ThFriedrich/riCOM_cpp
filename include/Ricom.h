#ifndef RICOM_H
#define RICOM_H

#include <stdio.h>
#include <cmath>
#include <complex>
#include <cfloat>
#include <vector>
#include <string>
#include "MerlinInterface.hpp"
#include "TimpixInterface.h"
#include <SDL.h>
#include <mutex>
#include <future>

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
    // Methods
    void compute_kernel();
    void compute_filter();
    void absorb_filter();
    std::vector<int> fftshift_map(int x, int y);
    // Constructor
    Ricom_kernel() : kernel_size(5), b_filter(false), kernel_filter_frequency{20, 1}, k_width_sym(0), rotation(0.0), kernel_x(), kernel_y()
    {
        compute_kernel();
    };
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
    id_x_y(): id(0), x(0), y(0), nx_ricom(0), ny_ricom(0), valid(true){};

    id_x_y(int id, int x, int y, int nx_ricom, int ny_ricom, bool valid){
        this->id = id;
        this->x = x;
        this->y = y;
        this->nx_ricom = nx_ricom;
        this->ny_ricom = ny_ricom;
        this->valid = valid;
    };

    friend id_x_y operator+(id_x_y &c1, const int& c2) 
    {
        id_x_y res = c1;
        res.id = c1.id + c2;
        res.y = res.id/c1.nx_ricom;
        res.x = res.id%c1.nx_ricom;
        res.valid = res.y >= 0 && res.y < c1.ny_ricom && res.x < c1.nx_ricom && res.x >= 0;
        return res;
    };
};

class Ricom_detector
{
public:
    // Properties
    std::array<float, 2> radius;
    std::array<float, 2> offset;
    size_t nx_cam;
    size_t ny_cam;
    std::vector<int> id_list;

    // Methods
    void compute_detector();
    void compute_detector(std::array<float, 2> &offset);
    // Constructor
    Ricom_detector() : radius{0.0, 0.0}, offset{127.5, 127.5}, nx_cam(256), ny_cam(256), id_list()
    {
        compute_detector();
    };
    // Destructor
    ~Ricom_detector(){};
};

class Ricom : public MerlinInterface, public TimpixInterface
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

    // Scan Area Variables
    int img_px;

    // Variables for potting in the SDL2 frame
    float ricom_max;
    float ricom_min;

    // Thread Synchronization Variables
    std::mutex ricom_mutex;
    std::mutex stem_mutex;
    std::mutex counter_mutex;

    // Private Methods - General
    void init_uv();
    template <typename T>
    void process_frames();
    void process_timepix_stream();
    void init_surface(unsigned int width, unsigned int height);
    void draw_pixel(SDL_Surface *surface, int x, int y, float val, int color_map);
    void reset_limits();
    void reset_file();
    std::vector<id_x_y> calculate_update_list();
    inline void rescales_recomputes(std::vector<std::future<void>> &futures);
    inline void rescales_recomputes();
    template <typename T>
    inline void skip_frames(int n_skip, std::vector<T> &data);

    // Private Methods - riCOM
    void icom(std::array<float, 2> &com, int x, int y);
    template <typename T>
    void com(std::vector<T> *data, std::array<float,2> &com, int *dose_sum);
    template <typename T>
    void read_com_merlin(std::vector<T> &data, std::array<float, 2> &com, int &dose_sum);
    void set_ricom_image_kernel(int idx, int idy);
    void set_ricom_pixel(id_x_y idr);
    void set_ricom_pixel(int idx, int idy);
    template <typename T>
    void com_icom(std::vector<T> data, int ix, int iy, int *dose_sum, std::array<float, 2> *com_xy_sum);

    // Private Methods - vSTEM
    template <typename T>
    void stem(std::vector<T> *data, size_t id_stem);
    void set_stem_pixel(size_t idx, size_t idy);

public:
    RICOM::modes mode;
    bool b_print2file;
    float update_dose_lowbound;
    bool update_offset;
    bool use_detector;
    bool b_recompute_detector;
    bool b_recompute_kernel;
    Ricom_detector detector;
    Ricom_kernel kernel;
    std::array<float, 2> offset;
    std::array<float, 2> com_public;
    int depth;
    std::vector<float> com_map_x;
    std::vector<float> com_map_y;

    RICOM::Detector_type detector_type;

    // Scan Area Variables
    int nx;
    int ny;
    int nxy;
    int rep;
    int fr_total;
    int skip_row;
    int skip_img;

    // Variables for progress and performance
    float fr_freq;  // Frequncy per frame
    float fr_count; // Count all Frames processed in an image
    float fr_count_total; // Count all Frames in a scanning session
    bool rescale_ricom;
    bool rescale_stem;
    bool rc_quit;
    
    SDL_Surface *srf_ricom; // Surface for the window;
    int ricom_cmap;
    SDL_Surface *srf_stem;  // Surface for the window;
    int stem_cmap;
    SDL_Surface *srf_cbed;  // Surface for the window;
    int cbed_cmap;

    // Public Methods
    void rescale_ricom_image();
    void rescale_stem_image();
    void run();
    void run_merlin();
    void run_timepix();
    void reset();
    template <typename T>
    void plot_cbed(std::vector<T> data);

    // Constructor
    Ricom() : stem_data(), stem_max(-FLT_MAX), stem_min(FLT_MAX), u(), v(), ricom_data(), update_list(), img_px(0), ricom_max(-FLT_MAX), ricom_min(FLT_MAX), ricom_mutex(), stem_mutex(), counter_mutex(), mode(RICOM::FILE), b_print2file(false), update_dose_lowbound(6), update_offset(true), use_detector(false), b_recompute_detector(false), b_recompute_kernel(false), detector(), kernel(), offset{127.5, 127.5}, com_public{0.0,0.0},  depth(1), com_map_x(), com_map_y(), detector_type(RICOM::MERLIN), nx(257), ny(256), nxy(0), rep(1), fr_total(0), skip_row(0), skip_img(0), fr_freq(0.0), fr_count(0.0), fr_count_total(0.0), rescale_ricom(false), rescale_stem(false), rc_quit(false), srf_ricom(NULL), ricom_cmap(9), srf_stem(NULL), stem_cmap(9), srf_cbed(NULL), cbed_cmap(9){};

    // Destructor
    ~Ricom();

};

#endif // __RICOM_H__