// Loosely Based on: https://github.com/lschw/fftw_cpp (GPL3 license)

#ifndef __FFTW2D_CPP__HH__
#define __FFTW2D_CPP__HH__

#include <vector>
#include <complex>
#include <fftw3.h>

/**
 * Class representing a 2D Fourier transform
 */
class FFT2D
{
public:
    const size_t N1; // Number of data points 1
    const size_t N2; // Number of data points 2
    const size_t N;  // Total number of data points = N1*N2

private:
    fftwf_plan plan_fw;
    fftwf_plan plan_bw;

public:
    /**
     * Setup Fourier transform
     * @param N1       Number of datapoints in first dim.
     * @param N2       Number of datapoints in second dim.
     */
    FFT2D(int N1, int N2) : N1(N1), N2(N2), N(N1 * N2)
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
    inline void fft(const std::vector<std::complex<float>> &in, std::vector<std::complex<float>> &out)
    {
        // Ensure in-place transformation
        if (in.data() != out.data())
        {
            memcpy(out.data(), in.data(), N * sizeof(std::complex<float>));
        }
        fftshift2D(out);
        fftwf_execute_dft(plan_fw,
                          reinterpret_cast<fftwf_complex *>(out.data()),
                          reinterpret_cast<fftwf_complex *>(out.data()));

        ifftshift2D(out);

        // Scale amplitude as FFTW calculates unscaled coefficients
        for (size_t i = 0; i < N; ++i)
        {
            out[i] /= N;
        }
    }

    /**
     * Calculate inverse Fourier transform
     * @param in   Input data
     * @param out  Fourier transformed output data
     *             If in == out, the transformation is done in-place
     */
    inline void ifft(std::vector<std::complex<float>> &in, std::vector<std::complex<float>> &out)
    {

        // Ensure in-place transformation
        if (in.data() != out.data())
        {
            memcpy(out.data(), in.data(), N * sizeof(std::complex<float>));
        }
        fftshift2D(out);
        fftwf_execute_dft(plan_bw,
                          reinterpret_cast<fftwf_complex *>(out.data()),
                          reinterpret_cast<fftwf_complex *>(out.data()));
        ifftshift2D(out);
    }

    /**
     * Shift frequency for FFT
     * @param in   Input data, the transformation is done in-place
     */
    inline void fftshift2D(std::vector<std::complex<float>> &data)
    {
        size_t xshift = N2 / 2;
        size_t yshift = N1 / 2;
        if (N2 % 2 != 0)
        {
            std::vector<std::complex<float>> out(N);
            for (size_t x = 0; x < N2; x++)
            {
                size_t outX = (x + xshift) % N2;
                for (size_t y = 0; y < N1; y++)
                {
                    size_t outY = (y + yshift) % N1;
                    out[outX + N2 * outY] = data[x + N2 * y];
                }
            }
            copy(out.begin(), out.end(), &data[0]);
        }
        else
        {
            for (size_t x = 0; x < N2; x++)
            {
                size_t outX = (x + xshift) % N2;
                for (size_t y = 0; y < yshift; y++)
                {
                    size_t outY = (y + yshift) % N1;
                    swap(data[outX + N2 * outY], data[x + N2 * y]);
                }
            }
        }
    }

    /**
     * I-Shift frequency for FFT (Invert fftshift)
     * @param in   Input data, the transformation is done in-place
     */
    inline void ifftshift2D(std::vector<std::complex<float>> &data)
    {
        size_t xshift = N2 / 2;
        if (N2 % 2 != 0)
        {
            xshift++;
        }
        size_t yshift = N1 / 2;
        if (N1 % 2 != 0)
        {
            yshift++;
        }
        if (N % 2 != 0)
        {
            std::vector<std::complex<float>> out(N);
            for (size_t x = 0; x < N2; x++)
            {
                size_t outX = (x + xshift) % N2;
                for (size_t y = 0; y < N1; y++)
                {
                    size_t outY = (y + yshift) % N1;
                    out[outX + N2 * outY] = data[x + N2 * y];
                }
            }
            copy(out.begin(), out.end(), &data[0]);
        }
        else
        {
            for (size_t x = 0; x < N2; x++)
            {
                size_t outX = (x + xshift) % N2;
                for (size_t y = 0; y < yshift; y++)
                {
                    size_t outY = (y + yshift) % N1;
                    swap(data[outX + N2 * outY], data[x + N2 * y]);
                }
            }
        }
    }

    /**
     * Cast a std::vector<float> into
     * a std::vector<std::complex<float>> with zero
     * imaginary parts
     * @param in   Input std::vector<float>
     * @param out  Output std::vector<std::complex<float>>
     */
    static void r2c(const std::vector<float> &f, std::vector<std::complex<float>> &c)
    {
        for (size_t v = 0; v < f.size(); v++)
        {
            c[v] = std::complex(f[v], 0.0f);
        }
    }

    /**
     * Cast a std::vector<std::complex<float>> into
     * a std::vector<float>, using the real parts
     * @param in   Input std::vector<std::complex<float>>
     * @param out  Output std::vector<float> (complex.real())
     */
    static void c2r(const std::vector<std::complex<float>> &c, std::vector<float> &f)
    {
        for (size_t v = 0; v < c.size(); v++)
        {
            f[v] = c[v].real();
        }
    }

    /**
     * Cast a std::vector<std::complex<float>> into
     * a std::vector<float>, using the absolute values
     * @param in   Input std::vector<std::complex<float>>
     * @param out  Output std::vector<float> (complex.real())
     */
    static void c2abs(const std::vector<std::complex<float>> &c, std::vector<float> &f)
    {
        for (size_t v = 0; v < c.size(); v++)
        {
            f[v] = std::abs(c[v]);
        }
    }

    /**
     * Cast a std::vector<float>, into
     * a std::vector<std::complex<float>> with zero
     * imaginary parts
     * @param in Input std::vector<float>
     * @return c Output std::vector<std::complex<float>>
     */
    static std::vector<std::complex<float>> r2c(const std::vector<float> &f)
    {
        std::vector<std::complex<float>> c(f.size());
        for (size_t v = 0; v < f.size(); v++)
        {
            c[v] = std::complex(f[v], 0.0f);
        }
        return c;
    }

    /**
     * Cast a std::vector<std::complex<float>> into
     * a std::vector<float>, using the real parts
     * @param in   Input std::vector<std::complex<float>>
     * @return f  Output std::vector<float> (complex.real())
     */
    static std::vector<float> c2r(const std::vector<std::complex<float>> &c)
    {
        std::vector<float> f(c.size());
        for (size_t v = 0; v < c.size(); v++)
        {
            f[v] = c[v].real();
        }
        return f;
    }
};

#endif
