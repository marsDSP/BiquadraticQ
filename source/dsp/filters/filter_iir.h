#pragma once

#ifndef BIQUADRATICQ_FILTER_IIR_H
#define BIQUADRATICQ_FILTER_IIR_H

#include <complex>
#include <algorithm>
#include <stdexcept>
#include <xsimd/xsimd.hpp>

namespace MarsDSP::Filters::inline Biquadratic
{
    enum class FilterType
    {
        LowPass,
        HighPass,
        Peaking,
        BandPass,
        LowShelf,
        HighShelf,
        BandShelf,
        AllPass,
        Notch,
    };

    template <typename T>
    class biquad
    {
    public:
        using SampleType = T;
        constexpr biquad() noexcept = default;

        constexpr biquad(const biquad&) noexcept = default;
        constexpr biquad(biquad&&) noexcept = default;

        constexpr biquad &operator=(const biquad&) noexcept = default;
        constexpr biquad &operator=(biquad&&) noexcept = default;

        constexpr biquad(SampleType a0, SampleType a1, SampleType a2,
                         SampleType b0, SampleType b1, SampleType b2) noexcept;

        constexpr biquad(const std::complex<T> &pole,
                         const std::complex<T> &zero);

        constexpr biquad(const std::complex<T> &poleI, const std::complex<T> &poleII,
                         const std::complex<T> &zeroI, const std::complex<T> &zeroII);

        ~biquad() = default;

        constexpr SampleType get_a0() const noexcept;
        constexpr SampleType get_a1() const noexcept;
        constexpr SampleType get_a2() const noexcept;
        constexpr SampleType get_b0() const noexcept;
        constexpr SampleType get_b1() const noexcept;
        constexpr SampleType get_b2() const noexcept;

        constexpr void set_a0(SampleType val) noexcept;
        constexpr void set_a1(SampleType val) noexcept;
        constexpr void set_a2(SampleType val) noexcept;
        constexpr void set_b0(SampleType val) noexcept;
        constexpr void set_b1(SampleType val) noexcept;
        constexpr void set_b2(SampleType val) noexcept;

        // state get/set
        constexpr SampleType get_w0() const noexcept;
        constexpr SampleType get_w1() const noexcept;
        constexpr void set_w0(SampleType val) noexcept;
        constexpr void set_w1(SampleType val) noexcept;

        constexpr SampleType tick(SampleType xn) noexcept;

        template <typename inIter, typename outIter>
        constexpr void batch(inIter begin, inIter end, outIter capacity);

        [[nodiscard]] constexpr bool stability() const noexcept;
        constexpr explicit operator bool() const noexcept;

        constexpr void reset() noexcept;
        constexpr void update_coefficients(SampleType a0, SampleType a1, SampleType a2,
                                           SampleType b0, SampleType b1, SampleType b2) noexcept;

    private:
        SampleType coeff_b2 {0};
        SampleType coeff_b1 {0};
        SampleType coeff_b0 {1};
        SampleType coeff_a2 {0};
        SampleType coeff_a1 {0};
        SampleType coeff_a0 {1};
        SampleType state_w0 {0};
        SampleType state_w1 {0};
    };

    template <typename T>
    constexpr biquad<T>::biquad(SampleType a0, SampleType a1, SampleType a2,
                                SampleType b0, SampleType b1, SampleType b2) noexcept :
                                coeff_b2(b2 / a0), coeff_b1(b1 / a0), coeff_b0(b0 / a0),
                                coeff_a2(a2 / a0), coeff_a1(a1 / a0), coeff_a0(1)
    {
    }

    template <typename T>
    constexpr biquad<T>::biquad(const std::complex<T> &pole, const std::complex<T> &zero) :
                                coeff_b2(0), coeff_b1(1), coeff_b0(-zero.real()),
                                coeff_a2(0), coeff_a1(-pole.real()), coeff_a0(1)
    {
        if (pole.imag() != 0)
        {
            throw std::runtime_error("Expecting real pole");
        }

        if (zero.imag() != 0)
        {
            throw std::runtime_error("Expecting real zero");
        }
    }

    template <typename T>
    constexpr biquad<T>::biquad(const std::complex<T> &poleI, const std::complex<T> &poleII,
                                const std::complex<T> &zeroI, const std::complex<T> &zeroII) :
                                coeff_b0(1), coeff_a0(1)
    {
        if (poleI.imag() != 0)
        {
            if (poleII != std::conj(poleI))
            {
                throw std::runtime_error("Expecting complex conjugate");
            }
            coeff_a1 = -2 * poleI.real();
            coeff_a2 = std::norm(poleI);
        } else
        {
            if (poleII.imag() != 0)
            {
                throw std::runtime_error("Expecting real number");
            }
            coeff_a1 = -(poleI.real() + poleII.real());
            coeff_a2 = poleI.real() * poleII.real();
        }

        if (zeroI.imag() != 0)
        {
            if (zeroII != std::conj(zeroI))
            {
                throw std::runtime_error("Expecting complex (zero) conjugate");
            }
            coeff_b1 = -2 * zeroI.real();
            coeff_b2 = std::norm(zeroI);
        } else
        {
            if (zeroII.imag() != 0)
            {
                throw std::runtime_error("Expecting real zero");
            }
            coeff_b1 = -(zeroI.real() + zeroII.real());
            coeff_b2 = zeroI.real() * zeroII.real();
        }
    }

    template<typename T>
    constexpr void biquad<T>::reset() noexcept
    {
        state_w0 = 0;
        state_w1 = 0;
    }

    template<typename T>
    constexpr void biquad<T>::update_coefficients(SampleType a0, SampleType a1, SampleType a2,
                                                   SampleType b0, SampleType b1, SampleType b2) noexcept
    {
        coeff_b2 = b2 / a0;
        coeff_b1 = b1 / a0;
        coeff_b0 = b0 / a0;
        coeff_a2 = a2 / a0;
        coeff_a1 = a1 / a0;
        coeff_a0 = 1;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::tick(SampleType xn) noexcept
    {
        auto yn = coeff_b0 * xn + state_w0;
        state_w0 = coeff_b1 * xn - coeff_a1 * yn + state_w1;
        state_w1 = coeff_b2 * xn - coeff_a2 * yn;
        return yn;
    }

    // in *theory* i should be able to parallelize N independent biquads
    // each filter in the array processes a channel
    template <typename T, std::size_t N>
    void tickSIMD(std::array<biquad<T>, N> &filters, T **channelData, int numSamples)
    {
        if (numSamples <= 0) return;

        // vars/aliases
        using BatchType = xsimd::batch<T>;
        constexpr auto SIMDSize = BatchType::size;

        // assert correct simd width so N works
        static_assert(N <= SIMDSize, "Channel count must fit in a SIMD register");

        // load all filter coeffs into registers
        alignas(xsimd::default_arch::alignment()) std::array<T, SIMDSize> b0_arr{}, b1_arr{}, b2_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<T, SIMDSize> a1_arr{}, a2_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<T, SIMDSize> w0_arr{}, w1_arr{};

        for (auto ch {0uz}; ch < N; ++ch)
        {
            b0_arr[ch] = filters[ch].get_b0();
            b1_arr[ch] = filters[ch].get_b1();
            b2_arr[ch] = filters[ch].get_b2();
            a1_arr[ch] = filters[ch].get_a1();
            a2_arr[ch] = filters[ch].get_a2();
            w0_arr[ch] = filters[ch].get_w0();
            w1_arr[ch] = filters[ch].get_w1();
        }

        // expose state getters
        auto b0 = BatchType::load_aligned(b0_arr.data());
        auto b1 = BatchType::load_aligned(b1_arr.data());
        auto b2 = BatchType::load_aligned(b2_arr.data());
        auto a1 = BatchType::load_aligned(a1_arr.data());
        auto a2 = BatchType::load_aligned(a2_arr.data());
        auto w0 = BatchType::load_aligned(w0_arr.data());
        auto w1 = BatchType::load_aligned(w1_arr.data());

        alignas(xsimd::default_arch::alignment()) std::array<T, SIMDSize> xn_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<T, SIMDSize> yn_arr{};

        for (int i {}; i < numSamples; ++i)
        {
            // grab 1 sample from each channel
            for (auto ch {0uz}; ch < N; ++ch)
                xn_arr[ch] = channelData[ch] ? channelData[ch][i] : 0;

            auto xn = BatchType::load_aligned(xn_arr.data());

            // transposed direct form II
            auto yn = xsimd::fma(b0, xn, w0);
            w0 = xsimd::fma(b1, xn, xsimd::fma(-a1, yn, w1)); // b1*xn - a1*yn + w1
            w1 = xsimd::fma(b2, xn, -a2 * yn);

            // back to each channel
            yn.store_aligned(yn_arr.data());
            for (auto ch {0uz}; ch < N; ++ch)
            {
                if (channelData[ch])
                    channelData[ch][i] = yn_arr[ch];
            }
        }
        // store state back
        w0.store_aligned(w0_arr.data());
        w1.store_aligned(w1_arr.data());

        for (std::size_t ch = 0; ch < N; ++ch)
        {
            filters[ch].set_w0(w0_arr[ch]);
            filters[ch].set_w1(w1_arr[ch]);
        }
    }

    template <typename T>
    template <typename inIter, typename outIter>
    constexpr void biquad<T>::batch(inIter begin, inIter end, outIter capacity)
    {
        for (; begin != end; ++begin, ++capacity)
        {
            *capacity = tick(*begin);
        }
    }

    template <typename T>
    constexpr bool biquad<T>::stability() const noexcept
    {
        return std::abs(coeff_a2) < 1 && std::abs(coeff_a1) < 1 + coeff_a2;
    }

    template <typename T>
    constexpr biquad<T>::operator bool() const noexcept
    {
        return stability();
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_w0() const noexcept
    {
        return state_w0;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_w1() const noexcept
    {
        return state_w1;
    }

    template <typename T>
    constexpr void biquad<T>::set_w0(SampleType val) noexcept
    {
        state_w0 = val;
    }

    template <typename T>
    constexpr void biquad<T>::set_w1(SampleType val) noexcept
    {
        state_w1 = val;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_a0() const noexcept
    {
        return coeff_a0;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_a1() const noexcept
    {
        return coeff_a1;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_a2() const noexcept
    {
        return coeff_a2;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_b0() const noexcept
    {
        return coeff_b0;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_b1() const noexcept
    {
        return coeff_b1;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_b2() const noexcept
    {
        return coeff_b2;
    }

    template <typename T>
    constexpr void biquad<T>::set_a0(const SampleType val) noexcept
    {
        coeff_a0 = val;
    }

    template <typename T>
    constexpr void biquad<T>::set_a1(const SampleType val) noexcept
    {
        coeff_a1 = val;
    }

    template <typename T>
    constexpr void biquad<T>::set_a2(const SampleType val) noexcept
    {
        coeff_a2 = val;
    }

    template <typename T>
    constexpr void biquad<T>::set_b0(const SampleType val) noexcept
    {
        coeff_b0 = val;
    }

    template <typename T>
    constexpr void biquad<T>::set_b1(const SampleType val) noexcept
    {
        coeff_b1 = val;
    }

    template <typename T>
    constexpr void biquad<T>::set_b2(const SampleType val) noexcept
    {
        coeff_b2 = val;
    }
}
#endif