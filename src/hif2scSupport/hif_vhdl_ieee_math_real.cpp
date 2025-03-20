/// @file hif_vhdl_ieee_math_real.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <stdint.h>

#include "hif2scSupport/hif_vhdl_ieee_math_real.hpp"
#include "hif2scSupport/hif_vhdl_standard.hpp"

namespace hif_vhdl_ieee_math_real
{

namespace /*anon*/
{
} // namespace

const long double math_e             = 2.71828182845904523536L;
const long double math_1_over_e      = 0.36787944117144232160L;
const long double math_pi            = 3.14159265358979323846L;
const long double math_2_pi          = 6.28318530717958647693L;
const long double math_1_over_pi     = 0.31830988618379067154L;
const long double math_pi_over_2     = 1.57079632679489661923L;
const long double math_pi_over_3     = 1.04719755119659774615L;
const long double math_pi_over_4     = 0.78539816339744830962L;
const long double math_log_of_2      = 0.69314718055994530942L;
const long double math_log_of_10     = 2.30258509299404568402L;
const long double math_log2_of_e     = 1.4426950408889634074L;
const long double math_log10_of_e    = 0.43429448190325182765L;
const long double math_sqrt_2        = 1.41421356237309504880L;
const long double math_1_over_sqrt_2 = 0.70710678118654752440L;
const long double math_sqrt_pi       = 1.77245385090551602730L;
const long double math_deg_to_rad    = 0.01745329251994329577L;
const long double math_rad_to_deg    = 57.29577951308232087685L;

double hif_vhdl_sign(double x)
{
    if (x > 0.0)
        return 1.0;
    else if (x < 0.0)
        return -1.0;
    else
        return 0.0;
}

void hif_vhdl_uniform(int &seed1, int &seed2, double &x)
{
    int32_t z, k;

    k     = seed1 / 53668;
    seed1 = 40014 * (seed1 - k * 53668) - k * 12211;

    if (seed1 < 0)
        seed1 = seed1 + 2147483563;

    k     = seed2 / 52774;
    seed2 = 40692 * (seed2 - k * 52774) - k * 3791;

    if (seed2 < 0)
        seed2 = seed2 + 2147483399;

    z = seed1 - seed2;
    if (z < 1)
        z = z + 2147483562;

    x = double(z) * 4.656613e-10;
}

long long int hif_vhdl_get_rand_max() { return RAND_MAX; }

double hif_vhdl_log(const double x)
{
    if (x <= 0.0) {
        hif_vhdl_standard::hif_vhdl_assert(false, "X <= 0.0 in LOG(X)", hif_vhdl_standard::hif_vhdl_error);
        return DBL_MIN;
    }

    return log(x);
}

double hif_vhdl_log2(const double x)
{
    if (x <= 0.0) {
        hif_vhdl_standard::hif_vhdl_assert(false, "X <= 0.0 in LOG2(X)", hif_vhdl_standard::hif_vhdl_error);
        return DBL_MIN;
    }

    return log2(x);
}

double hif_vhdl_log10(const double x)
{
    if (x <= 0.0) {
        hif_vhdl_standard::hif_vhdl_assert(false, "X <= 0.0 in LOG10(X)", hif_vhdl_standard::hif_vhdl_error);
        return DBL_MIN;
    }

    return log10(x);
}

double hif_vhdl_log(const int base, const double x)
{
    if ((base <= 0) || (x <= 0.0)) {
        hif_vhdl_standard::hif_vhdl_assert(
            false, "BASE <= 0 or X <= 0.0 in LOG(BASE, X)", hif_vhdl_standard::hif_vhdl_error);
        return DBL_MIN;
    }

    if (base == 2) {
#if (defined _MSC_VER)
        return log(x) / log(2.0);
#else
        return log2(x);
#endif
    } else if (base == 10)
        return log10(x);
    else if (fabsl(base - math_e) < 0.000000001L)
        return log(x);

    return (log(x) / log(double(base)));
}

} // namespace hif_vhdl_ieee_math_real
