/// @file hif_systemc_extensions.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <experimental/filesystem>

#include "hif2scSupport/hif_systemc_extensions.hpp"

namespace hif_systemc_extensions
{
namespace /*anon*/
{

#define Abs(x)    ((x) < 0 ? -(x) : (x))
#define Max(a, b) ((a) > (b) ? (a) : (b))

template <typename Real> inline constexpr bool is_real_equal(const Real v1, const Real v2, const Real tol)
{
    const auto abs1 = Abs(v1);
    const auto abs2 = Abs(v2);
    const auto max  = Max(abs1, abs2);
    const auto c    = Real(-1) / tol;
    return int(max * c) == 0 ? true : Abs(v1 - v2) / max <= tol;
}

std::string _resourcePath = "";

} // namespace

double hif_dTolerance       = 1e-09;
float hif_fTolerance        = 1e-09f;
long double hif_ldTolerance = 1e-09L;

bool hif_equals(const double v1, const double v2) { return is_real_equal(v1, v2, hif_dTolerance); }

bool hif_equals(const float v1, const float v2) { return is_real_equal(v1, v2, hif_fTolerance); }

bool hif_equals(const long double v1, const long double v2) { return is_real_equal(v1, v2, hif_ldTolerance); }

long long int hif_mod(const long long int a, const long long int n)
{
    // Note: if changing, check also SimplifyMap on IntValue,IntValue in simplify()
    if (a >= 0ll && n >= 0ll) {
        return a % n;
    } else if (a < 0ll && n < 0ll) {
        return -((-a) % (-n));
    } else if (a < 0ll && n >= 0ll) {
        return ((n - ((-a) % (n))) % n);
    } else //if(a >= 0 && n < 0)
    {
        return ((((a) % (-n)) + n) % n);
    }
}

bool hif_xorrd(const unsigned long long int v)
{
    unsigned long long int tmp = v;
    bool res                   = ((tmp & 1ULL) != 0ULL);
    tmp                        = tmp / 2ULL;
    while (tmp != 0) {
        res ^= ((tmp & 1ULL) != 0ULL);
        tmp = tmp / 2;
    }

    return res;
}

sc_dt::sc_logic hif_logicEquals(sc_dt::sc_logic param1, sc_dt::sc_logic param2)
{
    if (!param1.is_01() || !param2.is_01())
        return sc_dt::sc_logic('X');

    return sc_dt::sc_logic(param1 == param2);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
hdtlib::hl_logic_t hif_logicEquals_hdtlib(hdtlib::hl_logic_t param1, hdtlib::hl_logic_t param2)
{
    if (!param1.is_01() || !param2.is_01())
        return hdtlib::hl_logic_t('X');

    return hdtlib::hl_logic_t(param1 == param2);
}
#endif

sc_dt::sc_logic hif__op_lt(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return sc_dt::sc_logic('X');
    return sc_dt::sc_logic(v1.to_bool() < v2.to_bool());
}

sc_dt::sc_logic hif__op_gt(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return sc_dt::sc_logic('X');
    return sc_dt::sc_logic(v1.to_bool() > v2.to_bool());
}

sc_dt::sc_logic hif__op_le(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return sc_dt::sc_logic('X');
    return sc_dt::sc_logic(v1.to_bool() <= v2.to_bool());
}

sc_dt::sc_logic hif__op_ge(const sc_dt::sc_logic &v1, const sc_dt::sc_logic &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return sc_dt::sc_logic('X');
    return sc_dt::sc_logic(v1.to_bool() >= v2.to_bool());
}

bool hif_caseXZ(sc_dt::sc_logic param1, sc_dt::sc_logic param2, const bool param3)
{
    if (param1 == param2)
        return true;
    else if (param1 == 'Z' || param2 == 'Z')
        return true;
    else if (param3 && (param1 == 'X' || param2 == 'X'))
        return true;

    return false;
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
hdtlib::hl_logic_t hif__op_lt(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return v1.to_bool() < v2.to_bool();
}

hdtlib::hl_logic_t hif__op_gt(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return v1.to_bool() > v2.to_bool();
}

hdtlib::hl_logic_t hif__op_le(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return v1.to_bool() <= v2.to_bool();
}

hdtlib::hl_logic_t hif__op_ge(const hdtlib::hl_logic_t &v1, const hdtlib::hl_logic_t &v2)
{
    if (!v1.is_01() || !v2.is_01())
        return hdtlib::hl_logic_t('X');
    return v1.to_bool() >= v2.to_bool();
}

bool hif_caseXZ(hdtlib::hl_logic_t param1, hdtlib::hl_logic_t param2, const bool param3)
{
    if (param1 == param2)
        return true;
    else if (param1 == 'Z' || param2 == 'Z')
        return true;
    else if (param3 && (param1 == 'X' || param2 == 'X'))
        return true;

    return false;
}

#endif

// Reverse operations
std::string hif_reverse(const std::string &p)
{
    std::string s(p);
    std::reverse(s.begin(), s.end());
    return s;
}

void hif_setResourcePath(const std::string &path) { _resourcePath = path; }

std::string hif_getResourcePath() { return _resourcePath; }

std::string hif_getResourceFileName(const std::string &name)
{
    if (name.empty())
        return "";
    if (_resourcePath.empty())
        return name;

    std::experimental::filesystem::v1::path relPath(name);
    if (relPath.is_absolute())
        return name;

    std::experimental::filesystem::v1::path absPath(_resourcePath);
    absPath = absPath / name;

    const auto ok = std::experimental::filesystem::v1::exists(absPath);
    if (ok)
        return absPath;

    return name;
}

} // namespace hif_systemc_extensions
