#pragma once

#ifndef BIQUADRATICQ_FILTER_CONFIG_H
#define BIQUADRATICQ_FILTER_CONFIG_H

namespace MarsDSP::Filters::
inline Config
{
    template<typename T>
    struct constants
    {
        static constexpr T half                     = static_cast<T>(5.000000000000000000000000000000000000e-01);
        static constexpr T third                    = static_cast<T>(3.333333333333333333333333333333333333e-01);
        static constexpr T twothirds                = static_cast<T>(6.666666666666666666666666666666666666e-01);
        static constexpr T two_thirds               = static_cast<T>(6.666666666666666666666666666666666666e-01);
        static constexpr T sixth                    = static_cast<T>(1.66666666666666666666666666666666666666666e-01);
        static constexpr T three_quarters           = static_cast<T>(7.500000000000000000000000000000000000e-01);
        static constexpr T root_two                 = static_cast<T>(1.414213562373095048801688724209698078e+00);
        static constexpr T root_three               = static_cast<T>(1.732050807568877293527446341505872366e+00);
        static constexpr T half_root_two            = static_cast<T>(7.071067811865475244008443621048490392e-01);
        static constexpr T ln_two                   = static_cast<T>(6.931471805599453094172321214581765680e-01);
        static constexpr T ln_ln_two                = static_cast<T>(-3.665129205816643270124391582326694694e-01);
        static constexpr T root_ln_four             = static_cast<T>(1.177410022515474691011569326459699637e+00);
        static constexpr T one_div_root_two         = static_cast<T>(7.071067811865475244008443621048490392e-01);
        static constexpr T pi                       = static_cast<T>(3.141592653589793238462643383279502884e+00);
        static constexpr T half_pi                  = static_cast<T>(1.570796326794896619231321691639751442e+00);
        static constexpr T third_pi                 = static_cast<T>(1.047197551196597746154214461093167628e+00);
        static constexpr T sixth_pi                 = static_cast<T>(5.235987755982988730771072305465838140e-01);
        static constexpr T two_pi                   = static_cast<T>(6.283185307179586476925286766559005768e+00);
        static constexpr T two_thirds_pi            = static_cast<T>(2.094395102393195492308428922186335256e+00);
        static constexpr T three_quarters_pi        = static_cast<T>(2.356194490192344928846982537459627163e+00);
        static constexpr T four_thirds_pi           = static_cast<T>(4.188790204786390984616857844372670512e+00);
        static constexpr T one_div_two_pi           = static_cast<T>(1.591549430918953357688837633725143620e-01);
        static constexpr T one_div_root_two_pi      = static_cast<T>(3.989422804014326779399460599343818684e-01);
        static constexpr T root_pi                  = static_cast<T>(1.772453850905516027298167483341145182e+00);
        static constexpr T root_half_pi             = static_cast<T>(1.253314137315500251207882642405522626e+00);
        static constexpr T root_two_pi              = static_cast<T>(2.506628274631000502415765284811045253e+00);
        static constexpr T log_root_two_pi          = static_cast<T>(9.189385332046727417803297364056176398e-01);
        static constexpr T one_div_root_pi          = static_cast<T>(5.641895835477562869480794515607725858e-01);
        static constexpr T root_one_div_pi          = static_cast<T>(5.641895835477562869480794515607725858e-01);
        static constexpr T pi_minus_three           = static_cast<T>(1.415926535897932384626433832795028841e-01);
        static constexpr T four_minus_pi            = static_cast<T>(8.584073464102067615373566167204971158e-01);
        static constexpr T pi_pow_e                 = static_cast<T>(2.245915771836104547342715220454373502e+01);
        static constexpr T pi_sqr                   = static_cast<T>(9.869604401089358618834490999876151135e+00);
        static constexpr T pi_sqr_div_six           = static_cast<T>(1.644934066848226436472415166646025189e+00);
        static constexpr T pi_cubed                 = static_cast<T>(3.100627668029982017547631506710139520e+01);
    };
}
#endif