#pragma once

#ifndef BIQUADRATICQ_FILTER_CASCADE_H
#define BIQUADRATICQ_FILTER_CASCADE_H

#include <array>
#include <cstddef>
#include <stdexcept>
#include "filter_iir.h"

namespace MarsDSP::Filters::inline BiquadCascade {

    template <typename T, size_t N>
    class cascade
    {
    public:
        using SampleType = T;
        using Stage = biquad<T>;
        using SizeType = std::size_t;
        using ref = Stage&;
        using cref = const Stage&;
        using itr = Stage*;
        using citr = const Stage*;

        constexpr cascade() = default;
        ~cascade() = default;

        [[nodiscard]] constexpr SizeType size() const noexcept;
        static constexpr SizeType maxSize() noexcept;
        static constexpr SizeType capacity() noexcept;

        constexpr void clear();
        constexpr void reset();

        constexpr ref operator[](SizeType idx) noexcept;
        constexpr cref operator[](SizeType idx) const noexcept;

        constexpr itr begin() noexcept;
        constexpr itr end() noexcept;

        constexpr citr begin() const noexcept;
        constexpr citr cbegin() const noexcept;
        constexpr citr end() const noexcept;
        constexpr citr cend() const noexcept;

        constexpr SampleType tick(T xn) noexcept;

        template <typename inIter, typename outIter>
        constexpr void batch(inIter begin, inIter end, outIter capacity);

        constexpr void push_back(const biquad<T> &biquad);

        template <typename... Arg>
        constexpr void emplace_back(Arg... arg);

    private:
        std::size_t numStage {0};
        std::array<Stage, N> casc {};
    };

    template<typename T, size_t N>
    constexpr cascade<T, N>::SizeType cascade<T, N>::size() const noexcept
    {
        return numStage;
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::SizeType cascade<T, N>::maxSize() noexcept
    {
        return N;
    }

    template<typename T, size_t N>
    template<typename... Arg>
    constexpr void cascade<T, N>::emplace_back(Arg... arg)
    {
        if (numStage >= N)
        {
            throw std::runtime_error("No space available");
        }
        casc[numStage] = biquad<T>(arg...);
        numStage++;
    }

    template<typename T, size_t N>
    constexpr void cascade<T, N>::push_back(const biquad<T> &biquad)
    {
        if (numStage >= N)
        {
            throw std::runtime_error("No space available");
        }
        casc[numStage] = biquad;
        numStage++;
    }

    template<typename T, size_t N>
    constexpr T cascade<T, N>::tick(T xn) noexcept
    {
        for (auto i = 0ul; i < numStage; ++i)
        {
            xn = casc[i].tick(xn);
        }
        return xn;
    }

    template<typename T, size_t N>
    template <typename inIter, typename outIter>
    constexpr void cascade<T, N>::batch(inIter begin, inIter end, outIter capacity)
    {
        for (; begin != end; ++begin, ++capacity)
        {
            *capacity = tick(*begin);
        }
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::citr cascade<T, N>::end() const noexcept
    {
        return std::cbegin(casc) + size();
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::citr cascade<T, N>::cend() const noexcept
    {
        return std::cbegin(casc) + size();
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::citr cascade<T, N>::begin() const noexcept
    {
        return std::cbegin(casc);
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::citr cascade<T, N>::cbegin() const noexcept
    {
        return std::cbegin(casc);
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::itr cascade<T, N>::end() noexcept
    {
        return std::begin(casc) + size();
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::itr cascade<T, N>::begin() noexcept
    {
        return std::begin(casc);
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::ref cascade<T, N>::
    operator[](SizeType idx) noexcept
    {
        return casc[idx];
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::cref cascade<T, N>::
    operator[](SizeType idx) const noexcept
    {
        return casc[idx];
    }

    template<typename T, size_t N>
    constexpr void cascade<T, N>::reset()
    {
        for (auto i = 0ul; i < numStage; ++i)
        {
            casc[i].reset();
        }
    }

    template<typename T, size_t N>
    constexpr void cascade<T, N>::clear()
    {
        numStage = 0;
    }

    template<typename T, size_t N>
    constexpr cascade<T, N>::SizeType cascade<T, N>::capacity() noexcept
    {
        return N;
    }
}
#endif