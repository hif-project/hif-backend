/// @file hif_vhdl_ieee_math_real.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include "hif2scSupport/hif2scSupport/config.hpp"

namespace hif_vhdl_ieee_math_real
{

//constant  MATH_E :   real := 2.71828_18284_59045_23536;
HIF2SCSUPPORT_EXPORT
extern const long double math_e;

//constant  MATH_1_E:  real := 0.36787_94411_71442_32160;
HIF2SCSUPPORT_EXPORT
extern const long double math_1_over_e;

//constant  MATH_PI :  real := 3.14159_26535_89793_23846;
HIF2SCSUPPORT_EXPORT
extern const long double math_pi;

//constant  MATH_2_PI :  real := 6.28318_53071_79586_47693;
HIF2SCSUPPORT_EXPORT
extern const long double math_2_pi;

//constant  MATH_1_OVER_PI :  real := 0.31830_98861_83790_67154;
HIF2SCSUPPORT_EXPORT
extern const long double math_1_over_pi;

//constant  MATH_PI_OVER_2 :  real := 1.57079_63267_94896_61923;
HIF2SCSUPPORT_EXPORT
extern const long double math_pi_over_2;

//constant  MATH_PI_OVER_3 :  real := 1.04719_75511_96597_74615;
HIF2SCSUPPORT_EXPORT
extern const long double math_pi_over_3;

//constant  MATH_PI_OVER_4 :  real := 0.78539_81633_97448_30962;
HIF2SCSUPPORT_EXPORT
extern const long double math_pi_over_4;

//constant  MATH_LOG_OF_2:  real := 0.69314_71805_59945_30942;
HIF2SCSUPPORT_EXPORT
extern const long double math_log_of_2;

//constant  MATH_LOG_OF_10: real := 2.30258_50929_94045_68402;
HIF2SCSUPPORT_EXPORT
extern const long double math_log_of_10;

//constant  MATH_LOG2_OF_E:  real := 1.44269_50408_88963_4074;
HIF2SCSUPPORT_EXPORT
extern const long double math_log2_of_e;

//constant  MATH_LOG10_OF_E: real := 0.43429_44819_03251_82765;
HIF2SCSUPPORT_EXPORT
extern const long double math_log10_of_e;

//constant  MATH_SQRT_2: real := 1.41421_35623_73095_04880;
HIF2SCSUPPORT_EXPORT
extern const long double math_sqrt_2;

//constant  MATH_1_OVER_SQRT_2: real := 0.70710_67811_86547_52440;
HIF2SCSUPPORT_EXPORT
extern const long double math_1_over_sqrt_2;

//constant  MATH_SQRT_PI: real := 1.77245_38509_05516_02730;
HIF2SCSUPPORT_EXPORT
extern const long double math_sqrt_pi;

//constant  MATH_DEG_TO_RAD: real := 0.01745_32925_19943_29577;
HIF2SCSUPPORT_EXPORT
extern const long double math_deg_to_rad;

//constant  MATH_RAD_TO_DEG: real := 57.29577_95130_82320_87685;
HIF2SCSUPPORT_EXPORT
extern const long double math_rad_to_deg;

//function SIGN (X: real ) return real;
HIF2SCSUPPORT_EXPORT
double hif_vhdl_sign(double x);

//procedure UNIFORM (variable Seed1,Seed2:inout integer; variable X:out real)
HIF2SCSUPPORT_EXPORT
void hif_vhdl_uniform(int &seed1, int &seed2, double &x);

template <typename T1, typename T2> void hif_vhdl_uniform(T1 &seed1, T2 &seed2, double &x)
{
    int s1 = seed1.to_int();
    int s2 = seed2.to_int();
    hif_vhdl_uniform(s1, s2, x);
    seed1 = s1;
    seed2 = s2;
}

//function GET_RAND_MAX  return integer
HIF2SCSUPPORT_EXPORT
long long int hif_vhdl_get_rand_max();

//function LOG (real) return real;
HIF2SCSUPPORT_EXPORT
double hif_vhdl_log(const double x);

//function LOG2 (real) return real;
HIF2SCSUPPORT_EXPORT
double hif_vhdl_log2(const double x);

//function LOG10 (real) return real;
HIF2SCSUPPORT_EXPORT
double hif_vhdl_log10(const double x);

//function LOG (BASE: positive; X : real) return real;
HIF2SCSUPPORT_EXPORT
double hif_vhdl_log(const int base, const double x);

} // namespace hif_vhdl_ieee_math_real
