#pragma once

#ifndef BIQUADRATICQ_FILTER_BASE_H
#define BIQUADRATICQ_FILTER_BASE_H

#include <array>
#include <cassert>
#include <complex>

#include <juce_core/juce_core.h>

#ifndef expects
#define expects(cond, msg) jassert(cond)
#endif

#ifndef ensure
#define ensure(cond, msg) jassert(cond)
#endif

namespace MarsDSP::Filters::inline Base {
    inline namespace complex_ops
    {
        template<typename T>
        constexpr std::complex<T> addmul(const std::complex<T> &left,
                                         const T factor,
                                         const std::complex<T> &right) noexcept
        {
            return std::complex<T>(left.real() + factor * right.real(),
                                   left.imag() + factor * right.imag());
        }
    } // namespace complex_ops

    template<typename T>
    struct complex_pair : std::pair<std::complex<T>,
                std::complex<T> >
    {
        using base = std::pair<std::complex<T>,
            std::complex<T> >;

        explicit constexpr complex_pair(const std::complex<T> &c1) : base(c1, std::complex<T>(0, 0))
        {
            expects(c1.imag() == 0, "Expected a real number");
            ensure(this->is_real(), "Expected a real initialization");
        }

        constexpr complex_pair() : base(std::complex<T>(0, 0),
                                        std::complex<T>(0, 0))
        {
        }

        constexpr complex_pair(const std::complex<T> &c1,
                               const std::complex<T> &c2) : base(c1, c2)
        {
        }

        [[nodiscard]] constexpr bool isConjugate() const noexcept
        {
            return this->second == std::conj(this->first);
        }

        [[nodiscard]] constexpr bool is_real() const noexcept
        {
            return this->first.imag() == 0 &&
                   this->second.imag() == 0;
        }

        [[nodiscard]] constexpr bool is_matched_pair() const noexcept
        {
            if (this->first.imag() != 0)
            {
                return this->second == std::conj(this->first);
            }
            return this->second.imag() == 0 &&
                   this->second.real() != 0 &&
                   this->first.real() != 0;
        }

        [[nodiscard]] constexpr bool NaN() const noexcept
        {
            return std::isnan(this->first) || std::isnan(this->second);
        }
    };

    template<typename T>
    struct pz_pair : std::pair<complex_pair<T>,
                complex_pair<T> >
    {
        using base = std::pair<complex_pair<T>,
            complex_pair<T> >;

        constexpr pz_pair() : base(complex_pair<T>(),
                                   complex_pair<T>())
        {
        }

        constexpr pz_pair(const std::complex<T> &p,
                          const std::complex<T> &z) : base(p, z)
        {
        }

        constexpr pz_pair(const std::complex<T> &p1,
                          const std::complex<T> &z1,
                          const std::complex<T> &p2,
                          const std::complex<T> &z2) : base(complex_pair<T>(p1, p2),
                                                            complex_pair<T>(z1, z2))
        {
        }

        [[nodiscard]] constexpr bool single_pole() const noexcept
        {
            return poles().second == std::complex<T>(0, 0) && zeros().second == std::complex<T>(0, 0);
        }

        [[nodiscard]] constexpr bool NaN() const noexcept
        {
            return this->first.NaN() || this->second.NaN();
        }

        constexpr const complex_pair<T> &poles() const noexcept
        {
            return this->first;
        }

        constexpr complex_pair<T> &poles() noexcept
        {
            return this->first;
        }

        constexpr const complex_pair<T> &zeros() const noexcept
        {
            return this->second;
        }

        constexpr complex_pair<T> &zeros() noexcept
        {
            return this->second;
        }
    };

    template<typename T, std::size_t MaxSize = 50>
    struct LayoutBase
    {
        using value_type = pz_pair<T>;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;
        using iterator = value_type *;
        using const_iterator = const value_type *;

        constexpr LayoutBase() = default;

        constexpr T w() const noexcept
        {
            return normal_W_;
        }

        constexpr T gain() const noexcept
        {
            return normal_gain_;
        }

        [[nodiscard]] constexpr size_type poles() const noexcept
        {
            return num_poles_;
        }

        constexpr void set_w(T normal_W) noexcept
        {
            normal_W_ = normal_W;
        }

        constexpr void set_gain(T normal_gain) noexcept
        {
            normal_gain_ = normal_gain;
        }

        constexpr void insert(const std::complex<T> &pole,
                              const std::complex<T> &zero)
        {
            ensure_even_poles();

            expects(!is_nan(pole), "NAN number cannot be a pole");
            pairs_[pair_count()] = pz_pair<T>(pole, zero);

            num_poles_ += 1;
        }

        constexpr void insert_conjugate(const std::complex<T> &pole,
                                        const std::complex<T> &zero)
        {
            ensure_even_poles();

            expects(!is_nan(pole), "NAN number cannot be a pole");
            expects(!is_nan(zero), "NAN number cannot be a zero");

            pairs_[pair_count()] = pz_pair<T>(pole,
                                              zero,
                                              std::conj(pole),
                                              std::conj(zero));
            num_poles_ += 2;
        }

        constexpr void insert(const complex_pair<T> &poles,
                              const complex_pair<T> &zeros)
        {
            ensure_even_poles();

            expects(poles.is_matched_pair(), "Expected conjugate pairs");
            expects(zeros.is_matched_pair(), "Expected conjugate pairs");

            pairs_[pair_count()] = pz_pair<T>(poles.first,
                                              zeros.first,
                                              poles.second,
                                              zeros.second);
            num_poles_ += 2;
        }

        constexpr void reset() noexcept
        {
            num_poles_ = 0;
        }

        [[nodiscard]] constexpr size_type size() const noexcept
        {
            return (num_poles_ + 1) / 2 + 1;
        }

        constexpr const_iterator begin() const noexcept
        {
            return pairs_.data();
        }

        constexpr const_iterator end() const noexcept
        {
            return pairs_.data() + size();
        }

        constexpr const_reference operator[](size_type index) const noexcept
        {
            // no ensure
            if (index >= size())
            {
                throw std::runtime_error("Index out of bounds");
            }

            return pairs_[index];
        }

        constexpr const_reference at(size_type index) const
        {
            if (index >= size())
            {
                throw std::runtime_error("Index out of bounds");
            }

            return pairs_[index];
        }

    private:
        constexpr bool is_even(T x)
        {
            return !is_odd(x);
        }

        [[nodiscard]] constexpr size_type pair_count() const noexcept
        {
            return num_poles_ / 2;
        }

        constexpr void ensure_even_poles() const
        {
            assert(is_even(num_poles_));
        }

        size_type num_poles_{0};
        T normal_W_{0};
        T normal_gain_{1};
        std::array<value_type, MaxSize> pairs_{};
    };
}
#endif //BIQUADRATICQ_FILTER_BASE_H