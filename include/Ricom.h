#ifndef RICOM_H
#define RICOM_H

#include <stdio.h>
#include <cmath>
#include <vector>
#include <string>
#include "MerlinInterface.hpp"
#include <SDL.h>

class Ricom_kernel
{
public:
    // Properties
    int kernel_size;
    int k_width_sym;
    float rotation;
    std::vector<float> kernel_x;
    std::vector<float> kernel_y;
    // Methods
    void compute_kernel();
    // Constructor
    Ricom_kernel() : kernel_size(5), k_width_sym(0), rotation(0.0), kernel_x(), kernel_y(){
        compute_kernel();
    };
    // Destructor
    ~Ricom_kernel(){};
};

class Ricom_detector
{
public:
    // Properties
    std::array<float,2> radius;
    std::array<float,2> offset;
    size_t nx_merlin;
    size_t ny_merlin;
    std::vector<int> id_list;
    
    // Methods
    void compute_detector();
    void compute_detector(std::array<float,2> &offset);
    // Constructor
    Ricom_detector() : radius{0.0, 0.0}, offset{127.5, 127.5}, nx_merlin(256), ny_merlin(256), id_list(){
        compute_detector();
    };
    // Destructor
    ~Ricom_detector(){};
};


class Ricom : public MerlinInterface
{
private:
    // ADF Variables
    std::vector<float> stem_data;
    float stem_max;
    float stem_min;
    void set_stem_pixel(size_t idx, size_t idy);
    void rescale_stem_image();

    // ricom variables
    std::array<int, 256> u;
    std::array<int, 256> v;
    std::array<size_t, 256> sum_x;
    std::array<size_t, 256> sum_y;

    size_t nx;
    size_t ny;
    size_t nxy;
    size_t rep;
    int px_per_row;
    std::vector<float> ricom_data;

    // Variables for potting in the SDL2 frame
    float ricom_max;
    float ricom_min;

    // Private Methods
    template <typename T>
    void init_uv();
    void icom(std::array<float,2> &com, int x, int y, bool &rescale);
    template <typename T>
    void com(std::vector<T> &data, std::array<float,2> &com, size_t idx, bool &rescale);
    template <typename T>
    bool process_frames();
    void rescale_ricom_image();
    void set_ricom_image_kernel(int idx, int idy);
    void set_ricom_pixel(size_t idx, size_t idy);
    void init_surface(int height, int width);
    void draw_pixel(SDL_Surface *surface, int x, int y, float val);

public:
    bool use_detector;
    bool b_recompute_detector;
    bool b_recompute_kernel;
    Ricom_detector detector;
    Ricom_kernel kernel;
    std::array<float,2> offset;
    std::array<float,2> com_public;
    int depth;
    void run(size_t nx, size_t ny);
    // Variables for progress and performance
    float fr_freq;  // Frequncy per frame
    float fr_count; // Count all Frames processed
    bool rc_quit;
    void reset();
    template <typename T>
    void plot_cbed(std::vector<T> &data);

    SDL_Surface *srf_ricom;  // Surface for the window;
    SDL_Surface *srf_stem; // Surface for the window;
    SDL_Surface *srf_cbed; // Surface for the window;

    // Constructor
    Ricom() : stem_data(), stem_max(0.0), stem_min(INFINITY), u(), v(), sum_x{0}, sum_y{0}, nx(0), ny(0), nxy(0), rep(1), px_per_row(0), ricom_data(), ricom_max(0.0), ricom_min(INFINITY), use_detector(false), b_recompute_detector(false), b_recompute_kernel(false), detector(),  kernel(), offset{127.5, 127.5}, depth(1), fr_freq(0.0), fr_count(0), rc_quit(false), srf_ricom(NULL), srf_stem(NULL),srf_cbed(NULL){};

    // Destructor
    ~Ricom();
};

#endif // __RICOM_H__