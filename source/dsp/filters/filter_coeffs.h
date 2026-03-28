#pragma once

#ifndef BIQUADRATICQ_FILTER_COEFFS_H
#define BIQUADRATICQ_FILTER_COEFFS_H

#include <numbers>
#include "filter_iir.h"
#include "filter_utils.h"

namespace MarsDSP::Filters::
inline RBJ {

    template<typename T, FilterType Type>
    struct RBJ
    {
    };

    template <typename T>
    struct RBJ <T, FilterType::LowPass>
    {
        constexpr biquad<T> operator()(T sr, T Q, T cf, T gainDB = 1) const
        {
            unused(gainDB);
            const T new_cf = std::clamp(cf, static_cast<T>(10), static_cast<T>(sr * 0.49));
            const T newQ = std::max(Q, 10e-9);
            const T w0 = static_cast<T>(2) * std::numbers::pi_v<T> * (new_cf / sr);
            const T sinW0 = std::sin(w0);
            const T cosW0 = std::cos(w0);
            const T alpha = sinW0 / static_cast<T>(2) * newQ;
            const T omega = static_cast<T>(2) * std::pow(std::sin(w0 / static_cast<T>(2)), static_cast<T>(2));

            std::array<T, 3> a{}, b{};
            a[0] = static_cast<T>(1 + alpha);
            a[1] = static_cast<T>(-2 * cosW0);
            a[2] = static_cast<T>(1 - alpha);
            b[0] = static_cast<T>(omega / static_cast<T>(2));
            b[1] = static_cast<T>(omega);
            b[2] = b[0];
            return biquad<T>(a[0], a[1], a[2], b[0], b[1], b[2]);
        }
    };
}
#endif