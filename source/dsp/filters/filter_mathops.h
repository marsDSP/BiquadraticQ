#pragma once

#ifndef BIQUADRATICQ_FILTER_MATHOPS_H
#define BIQUADRATICQ_FILTER_MATHOPS_H

#include <cmath>
#include <algorithm>

namespace MarsDSP::Filters::
inline MathOps
{
    template <typename T>
    constexpr T fclamp(T val, T min, T max)
    {
        return std::clamp(val, min, max);
    }

    template <typename T>
    constexpr T fmax(T a, T b)
    {
        return (a > b) ? a : b;
    }

    template <typename T>
    T fsin(T val)
    {
        return std::sin(val);
    }
}
#endif