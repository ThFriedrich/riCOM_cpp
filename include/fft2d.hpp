// Based on: https://github.com/lschw/fftw_cpp (GPL3 license)

#ifndef __FFTW2D_CPP__HH__
#define __FFTW2D_CPP__HH__

#define _USE_MATH_DEFINES
#include <cmath>

#include <cstring>
#include <vector>
#include <complex>
#include <fftw3.h>

typedef std::complex<float> dcomplex;
typedef std::vector<float> dvector;
typedef std::vector<dcomplex> dcvector;

/**
 * Class representing a 2D Fourier transform
 */
class FFT2D
{
public:
    const size_t N1;           // Number of data points 1
    const float length1;      // Length of interval in real space 1
    const float sample_rate1; // Sample rate (N/length) 1
    const float df1;          // (Angular) frequency step (2*pi/length) 1
    const size_t N2;           // Number of data points 2
    const float length2;      // Length of interval in real space 2
    const float sample_rate2; // Sample rate (N/length) 2
    const float df2;          // (Angular) frequency step (2*pi/length) 2
    const size_t N;            // Total number of data points = N1*N2

private:
    fftwf_plan plan_fw;
    fftwf_plan plan_bw;

public:
    /**
     * Setup Fourier transform
     * @param N1       Number of datapoints in first dim.
     * @param N2       Number of datapoints in second dim.
     * @param length1  Length of interval in real space in first dim.
     * @param length2  Length of interval in real space in second dim.
     */
    FFT2D(size_t N1, size_t N2, float length1, float length2) : N1(N1), length1(length1),
                                                                  sample_rate1(N1 / length1), df1(2 * M_PI / length1),
                                                                  N2(N2), length2(length2),
                                                                  sample_rate2(N2 / length2), df2(2 * M_PI / length2),
                                                                  N(N1 * N2)
    {
        plan_fw = fftwf_plan_dft_2d(
            N1, N2, 0, 0, FFTW_FORWARD, FFTW_ESTIMATE);
        plan_bw = fftwf_plan_dft_2d(
            N1, N2, 0, 0, FFTW_BACKWARD, FFTW_ESTIMATE);
    }

    /**
     * Clean up
     */
    ~FFT2D()
    {
        fftwf_destroy_plan(plan_fw);
        fftwf_destroy_plan(plan_bw);
    }

    /**
     * Calculate Fourier transform
     * @param in   Input data
     * @param out  Fourier transformed output data
     *             If in == out, the transformation is done in-place
     */
    void fft(dcvector &in, dcvector &out)
    {
        dvector f1(N1);
        dvector f2(N2);
        freq1(f1);
        freq2(f2);
        shift_freq(f1, f2, in);

        // Ensure in-place transformation
        if (in.data() != out.data())
        {
            memcpy(out.data(), in.data(), N * sizeof(dcomplex));
        }
        fftwf_execute_dft(plan_fw,
                          reinterpret_cast<fftwf_complex *>(out.data()),
                          reinterpret_cast<fftwf_complex *>(out.data()));

        // Scale amplitude as FFTW calculates unscaled coefficients
        for (size_t i = 0; i < N; ++i)
        {
            out[i] /= N;
        }
        freq1(f1);
        freq2(f2);
        shift_freq(f1, f2, out);
    }

    /**
     * Calculate inverse Fourier transform
     * @param in   Input data
     * @param out  Fourier transformed output data
     *             If in == out, the transformation is done in-place
     */
    void ifft(dcvector &in, dcvector &out)
    {
        // Ensure in-place transformation
        if (in.data() != out.data())
        {
            memcpy(out.data(), in.data(), N * sizeof(dcomplex));
        }
        fftwf_execute_dft(plan_bw,
                          reinterpret_cast<fftwf_complex *>(out.data()),
                          reinterpret_cast<fftwf_complex *>(out.data()));
    }

    /**
     * Calculate sample frequencies (angular frequency)
     * @param f  This array will store the frequency data. Format:
     *           [0, df, 2*df, ..., N/2*df,
     *            -(N/2-1)*df, -(N/2-2)*df, ..., -df]
     */
    void _freq(dvector &f, size_t N, float sr)
    {
        f.resize(N);
        for (size_t i = 0; i < N; ++i)
        {
            if (i <= N / 2)
            {
                // Positive frequencies first
                f[i] = 2 * M_PI * i * sr / N;
            }
            else
            {
                f[i] = -2 * M_PI * (N - i) * sr / N;
            }
        }
    }
    void freq1(dvector &f)
    {
        return _freq(f, N1, sample_rate1);
    }
    void freq2(dvector &f)
    {
        return _freq(f, N2, sample_rate2);
    }

    /**
     * Shift frequency and data array to order frequencies from negative
     * to positive
     * @param f1     Frequency array1
     * @param f2     Frequency array2
     * @param data  Data array
     */
    void shift_freq(dvector &f1, dvector &f2, dcvector &data)
    {
        dvector buf1(N1);
        dvector buf2(N2);
        dcvector bufd(N);

        // Shift first dimension
        if (N1 % 2 == 0)
        { // Even number of data points
            for (size_t i = 0; i < N1 / 2 + 1; ++i)
            {
                buf1[N1 / 2 - 1 + i] = f1[i];

                for (size_t j = 0; j < N2; ++j)
                {
                    bufd[(N1 / 2 - 1 + i) * N2 + j] = data[i * N2 + j];
                }
                if (i < N1 / 2 - 1)
                {
                    buf1[i] = f1[N1 / 2 + 1 + i];
                    for (size_t j = 0; j < N2; ++j)
                    {
                        bufd[i * N2 + j] = data[(N1 / 2 + 1 + i) * N2 + j];
                    }
                }
            }
        }
        else
        { // Odd number of data points
            buf1[N1 / 2] = f1[0];
            bufd[N1 / 2] = data[0];
            for (size_t i = 0; i < N1 / 2; ++i)
            {
                buf1[N1 / 2 + 1 + i] = f1[i + 1];

                for (size_t j = 0; j < N2; ++j)
                {
                    bufd[(N1 / 2 + 1 + i) * N2 + j] = data[(i + 1) * N2 + j];
                }
                buf1[i] = f1[N1 / 2 + 1 + i];
                for (size_t j = 0; j < N2; ++j)
                {
                    bufd[i * N2 + j] = data[(N1 / 2 + 1 + i) * N2 + j];
                }
            }
        }

        // Shift second dimension
        if (N2 % 2 == 0)
        { // Even number of data points
            for (size_t i = 0; i < N2 / 2 + 1; ++i)
            {
                buf2[N2 / 2 - 1 + i] = f2[i];

                for (size_t j = 0; j < N1; ++j)
                {
                    data[j * N2 + N2 / 2 - 1 + i] = bufd[j * N2 + i];
                }
                if (i < N2 / 2 - 1)
                {
                    buf2[i] = f2[N2 / 2 + 1 + i];
                    for (size_t j = 0; j < N1; ++j)
                    {
                        data[j * N2 + i] = bufd[j * N2 + N2 / 2 + 1 + i];
                    }
                }
            }
        }
        else
        { // Odd number of data points
            buf2[N2 / 2] = f2[0];

            for (size_t j = 0; j < N1; ++j)
            {
                data[j * N2 + N2 / 2] = bufd[j * N2 + 0];
            }
            for (size_t i = 0; i < N2 / 2; ++i)
            {
                buf2[N2 / 2 + 1 + i] = f2[i + 1];

                for (size_t j = 0; j < N1; ++j)
                {
                    data[j * N2 + N2 / 2 + 1 + i] = bufd[j * N2 + i + 1];
                }
                buf2[i] = f2[N2 / 2 + 1 + i];
                for (size_t j = 0; j < N1; ++j)
                {
                    data[j * N2 + i] = bufd[j * N2 + N2 / 2 + 1 + i];
                }
            }
        }

        memcpy(f1.data(), buf1.data(), N1 * sizeof(float));
        memcpy(f2.data(), buf2.data(), N2 * sizeof(float));
    }

    static void r2c(dvector &f, dcvector &c)
    {
        for (size_t v = 0; v < f.size(); v++)
        {
            c[v] = std::complex(f[v], 0.0f);
        }
    }

    static void c2r(dcvector &c, dvector &f)
    {
        for (size_t v = 0; v < c.size(); v++)
        {
            f[v] = std::abs(c[v]);
        }
    }
};

#endif
