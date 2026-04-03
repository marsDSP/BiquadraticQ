#pragma once

#ifndef BIQUADRATICQ_FILTER_ENGINE_H
#define BIQUADRATICQ_FILTER_ENGINE_H

#include "filter_base.h"
#include "filter_iir.h"
#include "filter_cascade.h"
#include "filter_coeffs.h"
#include "filter_config.h"

namespace MarsDSP::Filters::Engine
{
    template<typename T, FilterType Type, typename... Args>
    biquad<T> calculate_coeffs(T sr, Args... args)
    {
        return Coeffs::CoeffCalc<T, Type>{}(sr, args...);
    }

    template<typename T, typename Container>
    void update_all(Container& filters, const biquad<T>& coeffs)
    {
        for (auto& filter : filters)
        {
            filter.update_coefficients(coeffs.get_a0(), coeffs.get_a1(), coeffs.get_a2(),
                                       coeffs.get_b0(), coeffs.get_b1(), coeffs.get_b2());
        }
    }

    template<typename T>
    biquad<T> make_biquad(const pz_pair<T> &pair) noexcept
    {
        if (pair.single_pole())
        {
            return Biquadratic::biquad<T>(pair.poles().first,
                                          pair.zeros().first);
        }
        return Biquadratic::biquad<T>(pair.poles().first,
                                      pair.zeros().first,
                                      pair.poles().second,
                                      pair.zeros().second);
    }

    template<typename T>
    void apply_scale(biquad<T> &biquad, T scale) noexcept
    {
        biquad.set_b0(biquad.get_b0() * scale);
        biquad.set_b1(biquad.get_b1() * scale);
        biquad.set_b2(biquad.get_b2() * scale);
    }

    template<typename T, std::size_t N>
    std::complex<T> make_response(const cascade<T, N> &cascade, T normalized_frequency)
    {
        const auto w = constants<T>::two_pi * normalized_frequency;
        const auto czn1 = std::polar(static_cast<T>(1), -w);
        const auto czn2 = std::polar(static_cast<T>(1), -2 * w);
        auto ch = std::complex<T>(1, 0);
        auto cbot = std::complex<T>(1, 0);

        for (std::int32_t i = static_cast<int32_t>(cascade.size()), index = 0; --i >= 0; ++index)
        {
            const auto &stage = cascade[index];
            auto cb = std::complex<T>(1, 0);
            auto ct = std::complex<T>(stage.get_b0() / stage.get_a0(), 0);

            ct = complex_ops::addmul(ct, stage.get_b1() / stage.get_a0(), czn1);
            ct = complex_ops::addmul(ct, stage.get_b2() / stage.get_a0(), czn2);
            cb = complex_ops::addmul(cb, stage.get_a1() / stage.get_a0(), czn1);
            cb = complex_ops::addmul(cb, stage.get_a2() / stage.get_a0(), czn2);
            ch *= ct;
            cbot *= cb;
        }
        return ch / cbot;
    }

    template<typename T, std::size_t N>
    constexpr cascade<T, (N + 1) / 2> make_cascade(const LayoutBase<T, N> &digital) noexcept
    {
        cascade<T, (N + 1) / 2> cascade;
        const auto num_poles = digital.poles();
        const auto num_biquads = (num_poles + 1) / 2;

        for (auto i{0ul}; i < num_biquads; ++i)
        {
            cascade.emplace_back(std::move(make_biquad(digital[i])));
        }

        const auto response = make_response(cascade, digital.w() / constants<T>::two_pi);
        const auto scale = digital.gain() / std::abs(response);
        apply_scale(cascade[0], scale);

        return cascade;
    }
}
#endif