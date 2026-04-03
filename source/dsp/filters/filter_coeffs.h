#pragma once

#ifndef BIQUADRATICQ_FILTER_COEFFS_H
#define BIQUADRATICQ_FILTER_COEFFS_H

#include <numbers>
#include <xsimd/xsimd.hpp>
#include "filter_iir.h"
#include "filter_utils.h"
#include "filter_mathops.h"

namespace MarsDSP::Filters::Coeffs
{
    template <typename T, FilterType Type>
    struct CoeffCalc
    {
    };

    template <typename T>
    struct CoeffCalc <T, FilterType::LowPass>
    {
        constexpr biquad<T> operator()(T sr, T cf, T Q, T gain = 1) const
        {
            unused(gain);
            const T newCF = fclamp(cf, static_cast<T>(10), static_cast<T>(sr * 0.49));
            const T newQ = fmax(Q, static_cast<T>(10e-9));
            const T w0 = (static_cast<T>(2) * std::numbers::pi_v<T>) * (newCF / sr);
            const auto [sinW0, cosW0] = xsimd::sincos(w0);
            const T alpha = sinW0 / (static_cast<T>(2) * newQ);
            const T omega = 1 - cosW0;

            std::array<T, 3> a{}, b{};
            a[0] = static_cast<T>(1+alpha);
            a[1] = static_cast<T>(-2*cosW0);
            a[2] = static_cast<T>(1-alpha);
            b[0] = static_cast<T>(omega/static_cast<T>(2));
            b[1] = static_cast<T>(omega);
            b[2] = b[0];
            return biquad<T>(a[0], a[1], a[2], b[0], b[1], b[2]);
        }
    };

    template <typename T>
    struct CoeffCalc <T, FilterType::HighPass>
    {
        constexpr biquad<T> operator()(T sr, T cf, T Q, T gain = 1) const
        {
            unused(gain);
            const T newCF = fclamp(cf, static_cast<T>(10), static_cast<T>(sr * 0.49));
            const T newQ = fmax(Q, static_cast<T>(10e-9));
            const T w0 = (static_cast<T>(2) * std::numbers::pi_v<T>) * (newCF / sr);
            const auto [sinW0, cosW0] = xsimd::sincos(w0);
            const T alpha = sinW0 / (static_cast<T>(2) * newQ);

            const T onePlusCos = 1 + cosW0;

            std::array<T, 3> a{}, b{};
            a[0] = static_cast<T>(1+alpha);
            a[1] = static_cast<T>(-2*cosW0);
            a[2] = static_cast<T>(1-alpha);
            b[0] = static_cast<T>(onePlusCos/static_cast<T>(2));
            b[1] = static_cast<T>(-(onePlusCos));
            b[2] = b[0];
            return biquad<T>(a[0], a[1], a[2], b[0], b[1], b[2]);
        }
    };

    template <typename T>
    struct CoeffCalc <T, FilterType::BandPass>
    {
        constexpr biquad<T> operator()(T sr, T centerFreqParam, T bandwidthParam, T gain = 1) const
        {
            unused(gain);
            const T halfSr = static_cast<T>(sr * 0.49);
            const T minFreq = static_cast<T>(10);
            
            const T centerFreq = fclamp(centerFreqParam, minFreq, halfSr);
            const T bw = fmax(bandwidthParam, static_cast<T>(10e-9));
            const T newQ = fmax(static_cast<T>(1000) / bw, static_cast<T>(10e-9));

            const T w0 = (static_cast<T>(2) * std::numbers::pi_v<T>) * (centerFreq / sr);
            const auto [sinW0, cosW0] = xsimd::sincos(w0);
            const T alpha = sinW0 / (static_cast<T>(2) * newQ);

            std::array<T, 3> a{}, b{};
            a[0] = static_cast<T>(1+alpha);
            a[1] = static_cast<T>(-2*cosW0);
            a[2] = static_cast<T>(1-alpha);
            b[0] = static_cast<T>(alpha);
            b[1] = static_cast<T>(0);
            b[2] = static_cast<T>(-alpha);
            return biquad<T>(a[0], a[1], a[2], b[0], b[1], b[2]);
        }
    };

    template <typename T>
    struct CoeffCalc <T, FilterType::AllPass>
    {
        constexpr biquad<T> operator()(T sr, T cf, T Q, T gain = 1) const
        {
            unused(gain);
            const T newCF = fclamp(cf, static_cast<T>(10), static_cast<T>(sr * 0.49));
            const T newQ = fmax(Q, static_cast<T>(10e-9));
            const T w0 = (static_cast<T>(2) * std::numbers::pi_v<T>) * (newCF / sr);
            const auto [sinW0, cosW0] = xsimd::sincos(w0);
            const T alpha = sinW0 / (static_cast<T>(2) * newQ);
            const T omega = 1 - cosW0;

            std::array<T, 3> a{}, b{};
            a[0] = static_cast<T>(1+alpha);
            a[1] = static_cast<T>(-2*cosW0);
            a[2] = static_cast<T>(1-alpha);
            b[0] = static_cast<T>(1-alpha);
            b[1] = static_cast<T>(-2*cosW0);
            b[2] = static_cast<T>(1+alpha);
            return biquad<T>(a[0], a[1], a[2], b[0], b[1], b[2]);
        }
    };
}
#endif