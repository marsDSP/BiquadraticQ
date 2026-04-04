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

        constexpr SampleType get_step_b0() const noexcept;
        constexpr SampleType get_step_b1() const noexcept;
        constexpr SampleType get_step_b2() const noexcept;
        constexpr SampleType get_step_a1() const noexcept;
        constexpr SampleType get_step_a2() const noexcept;
        constexpr int get_remaining_steps() const noexcept;
        constexpr void set_remaining_steps(int steps) noexcept;
        constexpr void set_coefficients_no_step(SampleType b0, SampleType b1, SampleType b2, SampleType a1, SampleType a2) noexcept;

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

        constexpr void smooth_coefficients(SampleType a0, SampleType a1, SampleType a2,
                                           SampleType b0, SampleType b1, SampleType b2,
                                           int steps) noexcept;

    private:
        SampleType coeff_b2 {0};
        SampleType coeff_b1 {0};
        SampleType coeff_b0 {1};
        SampleType coeff_a2 {0};
        SampleType coeff_a1 {0};
        SampleType coeff_a0 {1};

        SampleType target_b2 {0};
        SampleType target_b1 {0};
        SampleType target_b0 {1};
        SampleType target_a2 {0};
        SampleType target_a1 {0};

        SampleType step_b2 {0};
        SampleType step_b1 {0};
        SampleType step_b0 {0};
        SampleType step_a2 {0};
        SampleType step_a1 {0};

        int remaining_steps {0};

        double state_w0 {0};
        double state_w1 {0};
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

        target_b2 = coeff_b2;
        target_b1 = coeff_b1;
        target_b0 = coeff_b0;
        target_a2 = coeff_a2;
        target_a1 = coeff_a1;
        remaining_steps = 0;
    }

    template<typename T>
    constexpr void biquad<T>::smooth_coefficients(SampleType a0, SampleType a1, SampleType a2,
                                                   SampleType b0, SampleType b1, SampleType b2,
                                                   int steps) noexcept
    {
        if (steps <= 0)
        {
            update_coefficients(a0, a1, a2, b0, b1, b2);
            return;
        }

        target_b2 = b2 / a0;
        target_b1 = b1 / a0;
        target_b0 = b0 / a0;
        target_a2 = a2 / a0;
        target_a1 = a1 / a0;

        step_b2 = (target_b2 - coeff_b2) / static_cast<SampleType>(steps);
        step_b1 = (target_b1 - coeff_b1) / static_cast<SampleType>(steps);
        step_b0 = (target_b0 - coeff_b0) / static_cast<SampleType>(steps);
        step_a2 = (target_a2 - coeff_a2) / static_cast<SampleType>(steps);
        step_a1 = (target_a1 - coeff_a1) / static_cast<SampleType>(steps);

        remaining_steps = steps;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::tick(SampleType xn) noexcept
    {
        if (remaining_steps > 0)
        {
            coeff_b2 += step_b2;
            coeff_b1 += step_b1;
            coeff_b0 += step_b0;
            coeff_a2 += step_a2;
            coeff_a1 += step_a1;
            --remaining_steps;

            if (remaining_steps == 0)
            {
                coeff_b2 = target_b2;
                coeff_b1 = target_b1;
                coeff_b0 = target_b0;
                coeff_a2 = target_a2;
                coeff_a1 = target_a1;
            }
        }

        double xn_d = static_cast<double>(xn);
        double yn_d = static_cast<double>(coeff_b0) * xn_d + state_w0;
        yn_d = std::clamp(yn_d, static_cast<double>(-100.0), static_cast<double>(100.0));
        double next_w0 = static_cast<double>(coeff_b1) * xn_d - static_cast<double>(coeff_a1) * yn_d + state_w1;
        state_w1 = static_cast<double>(coeff_b2) * xn_d - static_cast<double>(coeff_a2) * yn_d;
        state_w0 = std::clamp(next_w0, static_cast<double>(-100.0), static_cast<double>(100.0));
        state_w1 = std::clamp(state_w1, static_cast<double>(-100.0), static_cast<double>(100.0));
        return static_cast<SampleType>(yn_d);
    }

    // in *theory* i should be able to parallelize N independent biquads
    // each filter in the array processes a channel
    template <typename FilterT, typename SampleT, std::size_t N>
    void tickSIMD(std::array<biquad<FilterT>, N> &filters, SampleT **channelData, int numSamples)
    {
        if (numSamples <= 0) return;

        // vars/aliases
        using BatchType = xsimd::batch<FilterT>;
        constexpr auto SIMDSize = BatchType::size;

        // assert correct simd width so N works
        static_assert(N <= SIMDSize, "Channel count must fit in a SIMD register");

        // load all filter coeffs into registers
        alignas(xsimd::default_arch::alignment()) std::array<FilterT, SIMDSize> b0_arr{}, b1_arr{}, b2_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<FilterT, SIMDSize> a1_arr{}, a2_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<FilterT, SIMDSize> w0_arr{}, w1_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<FilterT, SIMDSize> step_b0_arr{}, step_b1_arr{}, step_b2_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<FilterT, SIMDSize> step_a1_arr{}, step_a2_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<int, SIMDSize> rem_steps_arr{};

        for (auto ch {0uz}; ch < N; ++ch)
        {
            b0_arr[ch] = filters[ch].get_b0();
            b1_arr[ch] = filters[ch].get_b1();
            b2_arr[ch] = filters[ch].get_b2();
            a1_arr[ch] = filters[ch].get_a1();
            a2_arr[ch] = filters[ch].get_a2();
            w0_arr[ch] = static_cast<FilterT>(filters[ch].get_w0());
            w1_arr[ch] = static_cast<FilterT>(filters[ch].get_w1());

            step_b0_arr[ch] = filters[ch].get_step_b0();
            step_b1_arr[ch] = filters[ch].get_step_b1();
            step_b2_arr[ch] = filters[ch].get_step_b2();
            step_a1_arr[ch] = filters[ch].get_step_a1();
            step_a2_arr[ch] = filters[ch].get_step_a2();
            rem_steps_arr[ch] = filters[ch].get_remaining_steps();
        }

        // expose state getters
        auto b0 = BatchType::load_aligned(b0_arr.data());
        auto b1 = BatchType::load_aligned(b1_arr.data());
        auto b2 = BatchType::load_aligned(b2_arr.data());
        auto a1 = BatchType::load_aligned(a1_arr.data());
        auto a2 = BatchType::load_aligned(a2_arr.data());
        auto w0 = BatchType::load_aligned(w0_arr.data());
        auto w1 = BatchType::load_aligned(w1_arr.data());

        auto step_b0 = BatchType::load_aligned(step_b0_arr.data());
        auto step_b1 = BatchType::load_aligned(step_b1_arr.data());
        auto step_b2 = BatchType::load_aligned(step_b2_arr.data());
        auto step_a1 = BatchType::load_aligned(step_a1_arr.data());
        auto step_a2 = BatchType::load_aligned(step_a2_arr.data());

        alignas(xsimd::default_arch::alignment()) std::array<FilterT, SIMDSize> xn_arr{};
        alignas(xsimd::default_arch::alignment()) std::array<FilterT, SIMDSize> yn_arr{};

        for (int i {}; i < numSamples; ++i)
        {
            // update coefficients if smoothing is active
            // we check if any channel has remaining steps
            bool any_smoothing = false;
            for(auto ch=0uz; ch<N; ++ch) if(rem_steps_arr[ch] > 0) { any_smoothing = true; break; }

            if (any_smoothing)
            {
                b0 += step_b0;
                b1 += step_b1;
                b2 += step_b2;
                a1 += step_a1;
                a2 += step_a2;

                for(auto ch=0uz; ch<N; ++ch) 
                {
                    if(rem_steps_arr[ch] > 0) --rem_steps_arr[ch];
                    // If a channel reaches 0, it should ideally snap to target, 
                    // but for simplicity and since we use small steps it's usually fine to just stop.
                    // Actually, to be precise, we should zero out its step in the SIMD vector.
                    if(rem_steps_arr[ch] == 0)
                    {
                        step_b0_arr[ch] = 0;
                        step_b1_arr[ch] = 0;
                        step_b2_arr[ch] = 0;
                        step_a1_arr[ch] = 0;
                        step_a2_arr[ch] = 0;
                    }
                }
                step_b0 = BatchType::load_aligned(step_b0_arr.data());
                step_b1 = BatchType::load_aligned(step_b1_arr.data());
                step_b2 = BatchType::load_aligned(step_b2_arr.data());
                step_a1 = BatchType::load_aligned(step_a1_arr.data());
                step_a2 = BatchType::load_aligned(step_a2_arr.data());
            }

            // grab 1 sample from each channel
            for (auto ch {0uz}; ch < N; ++ch)
                xn_arr[ch] = channelData[ch] ? static_cast<FilterT>(channelData[ch][i]) : 0;

            auto xn = BatchType::load_aligned(xn_arr.data());

            auto yn = xsimd::fma(b0, xn, w0);
            
            // clamping to avoid explosions
            yn = xsimd::min(static_cast<BatchType>(100), xsimd::max(static_cast<BatchType>(-100), yn));

            auto next_w0 = xsimd::fma(b1, xn, xsimd::fma(-a1, yn, w1)); // b1*xn - a1*yn + w1
            auto next_w1 = xsimd::fma(b2, xn, -a2 * yn);
            
            w0 = xsimd::min(static_cast<BatchType>(100), xsimd::max(static_cast<BatchType>(-100), next_w0));
            w1 = xsimd::min(static_cast<BatchType>(100), xsimd::max(static_cast<BatchType>(-100), next_w1));

            // back to each channel
            yn.store_aligned(yn_arr.data());
            for (auto ch {0uz}; ch < N; ++ch)
            {
                if (channelData[ch])
                    channelData[ch][i] = static_cast<SampleT>(yn_arr[ch]);
            }
        }
        // store state back
        w0.store_aligned(w0_arr.data());
        w1.store_aligned(w1_arr.data());

        for (std::size_t ch = 0; ch < N; ++ch)
        {
            filters[ch].set_w0(static_cast<double>(w0_arr[ch]));
            filters[ch].set_w1(static_cast<double>(w1_arr[ch]));
        }

        // store coefficients back if they were being smoothed
        b0.store_aligned(b0_arr.data());
        b1.store_aligned(b1_arr.data());
        b2.store_aligned(b2_arr.data());
        a1.store_aligned(a1_arr.data());
        a2.store_aligned(a2_arr.data());

        for (std::size_t ch = 0; ch < N; ++ch)
        {
            filters[ch].set_coefficients_no_step(b0_arr[ch], b1_arr[ch], b2_arr[ch], a1_arr[ch], a2_arr[ch]);
            filters[ch].set_remaining_steps(rem_steps_arr[ch]);
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
        // stability condition for biquad:
        // |a2| < 1
        // |a1| < 1 + a2
        return std::abs(static_cast<double>(coeff_a2)) < 0.99999999 && 
               std::abs(static_cast<double>(coeff_a1)) < (1.0 + static_cast<double>(coeff_a2));
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

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_step_b0() const noexcept
    {
        return step_b0;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_step_b1() const noexcept
    {
        return step_b1;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_step_b2() const noexcept
    {
        return step_b2;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_step_a1() const noexcept
    {
        return step_a1;
    }

    template <typename T>
    constexpr biquad<T>::SampleType biquad<T>::get_step_a2() const noexcept
    {
        return step_a2;
    }

    template <typename T>
    constexpr int biquad<T>::get_remaining_steps() const noexcept
    {
        return remaining_steps;
    }

    template <typename T>
    constexpr void biquad<T>::set_remaining_steps(int steps) noexcept
    {
        remaining_steps = steps;
        if (remaining_steps <= 0)
        {
            step_b0 = step_b1 = step_b2 = step_a1 = step_a2 = 0;
        }
    }

    template <typename T>
    constexpr void biquad<T>::set_coefficients_no_step(SampleType b0, SampleType b1, SampleType b2, SampleType a1, SampleType a2) noexcept
    {
        coeff_b0 = b0;
        coeff_b1 = b1;
        coeff_b2 = b2;
        coeff_a1 = a1;
        coeff_a2 = a2;
    }
}
#endif