#pragma once

#ifndef BIQUADRATICQ_FILTER_UTILS_H
#define BIQUADRATICQ_FILTER_UTILS_H

#include <cassert>
#include <string_view>

#if __has_include(<nonstd/string_view.hpp>)
#include <nonstd/string_view.hpp>
#elif __has_include("nonstd/string_view.hpp")
#include "nonstd/string_view.hpp"
#else
#error "nonstd::string_view header is required by filter_utils.h"
#endif

namespace MarsDSP::Filters::
inline utils
{
    template<typename T>
    constexpr void unused(const T &variable)
    {
        (void) variable;
    }

    template<typename T>
    constexpr void unused(const T &variable, const nonstd::string_view &message)
    {
        unused(variable);
        unused(message);
    }

    inline void ensure(const bool condition)
    {
#if defined(NDEBUG)
        unused(condition);
#endif
        assert(condition);
    }
}
#endif