/// @file hif_verilog_standard.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include "hif2scSupport/hif_verilog_standard.hpp"

#ifdef __GNUC__
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7))
#pragma GCC diagnostic warning "-Wuseless-cast"
#endif
#endif
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

namespace hif_verilog_standard
{

namespace /* anon */
{

typedef std::map<int, FILE *> IOMap;
IOMap _ioMap;

void _hif_verilog__system_vfdisplay(const int fd, const char *const s, va_list argp)
{
    if (_ioMap.find(fd) == _ioMap.end()) {
#ifdef _MSC_VER
        FILE *f = _fdopen(fd, "a");
#else
        FILE *f = fdopen(fd, "a");
#endif
        if (f == nullptr)
            exit(EXIT_FAILURE);
        _ioMap[fd] = f;
    }
    FILE *f = _ioMap[fd];
    vfprintf(f, s, argp);
}

int _hif_verilog__system_vfscanf(const int fd, const char *const s, va_list argp)
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    if (_ioMap.find(fd) == _ioMap.end()) {
#ifdef _MSC_VER
        FILE *f = _fdopen(fd, "r+");
#else
        FILE *f = fdopen(fd, "r+");
#endif
        if (f == nullptr)
            exit(EXIT_FAILURE);
        _ioMap[fd] = f;
    }
    FILE *f = _ioMap[fd];

    return vfscanf(f, s, argp);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

} // namespace

// ///////////////////////////////////////////////////////////////////
// System tasks (Clause 17)
// ///////////////////////////////////////////////////////////////////

namespace internal
{

sc_dt::sc_lv<32> hif_verilog__system_getParam(sc_dt::sc_lv<32> p) { return p; }

void hif_verilog__system_finish_impl(const char *f, const int l, const char *func, sc_dt::sc_lv<32> param1)
{
    if (param1.to_int() != 0) {
        std::clog << "In " << f << ":" << l << "\nInside function " << func << "()\n";
    }

    fflush(nullptr);
    sc_core::sc_stop();
    //exit(0);
    //throw "EXIT";
}

void hif_verilog__system_stop_impl(const char *f, const int l, const char *func, sc_dt::sc_lv<32> param1)
{
    if (param1.to_int() != 0) {
        std::clog << "In " << f << ":" << l << "\nInside function " << func << "()\n";
    }

    sc_core::sc_stop();
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
hdtlib::hl_lv_t<32> hif_verilog__system_getParam_hdtlib(hdtlib::hl_lv_t<32> p) { return p; }

void hif_verilog__system_finish_impl(const char *f, const int l, const char *func, hdtlib::hl_lv_t<32> param1)
{
    if (param1.to_int() != 0) {
        std::clog << "In " << f << ":" << l << "\nInside function " << func << "()\n";
    }

    fflush(nullptr);
    sc_core::sc_stop();
    //exit(0);
}

void hif_verilog__system_stop_impl(const char *f, const int l, const char *func, hdtlib::hl_lv_t<32> param1)
{
    if (param1.to_int() != 0) {
        std::clog << "In " << f << ":" << l << "\nInside function " << func << "()\n";
    }

    sc_core::sc_stop();
}
#endif

} // namespace internal

sc_dt::sc_lv<32> hif_verilog__system_stime()
{
    return sc_dt::sc_lv<32>(static_cast<uint32_t>(sc_core::sc_time_stamp().to_double()));
}

sc_dt::sc_lv<64> hif_verilog__system_time()
{
    return sc_dt::sc_lv<64>(static_cast<uint64_t>(sc_core::sc_time_stamp().to_double()));
}

double hif_verilog__system_realtime() { return sc_core::sc_time_stamp().to_double(); }

int32_t hif_verilog__system_random(const uint64_t seed)
{
    if (seed != static_cast<uint64_t>(-1)) {
        srand(static_cast<unsigned int>(seed));
    }

    return rand();
}

void hif_verilog__system_fdisplay(const int fd, const char *const s, ...)
{
    va_list argp;
    va_start(argp, s);
    _hif_verilog__system_vfdisplay(fd, s, argp);
    va_end(argp);
}

void hif_verilog__system_fscanf(const int fd, const char *const s, ...)
{
    va_list argp;
    va_start(argp, s);
    _hif_verilog__system_vfscanf(fd, s, argp);
    va_end(argp);
}

void hif_verilog__system_fwrite(const int fd, const char *const s, ...)
{
    va_list argp;
    va_start(argp, s);
    _hif_verilog__system_vfdisplay(fd, s, argp);
    va_end(argp);
}

void hif_verilog__system_fflush(const int fd)
{
    if (_ioMap.find(fd) == _ioMap.end())
        return;
    FILE *f = _ioMap[fd];
    fflush(f);
}

void hif_verilog__system_fclose(const int fd)
{
    IOMap::iterator it = _ioMap.find(fd);
    if (it == _ioMap.end())
        return;
    FILE *f = it->second;
    fclose(f);
    _ioMap.erase(it);
}

int hif_verilog__system_feof(const int fd)
{
    if (_ioMap.find(fd) == _ioMap.end())
        return 0;
    FILE *f = _ioMap[fd];
    return feof(f);
}

#ifdef HIF2SCSUPPORT_USE_HDTLIB
hdtlib::hl_lv_t<32> hif_verilog__system_stime_hdtlib()
{
    return hdtlib::hl_lv_t<32>(static_cast<uint32_t>(sc_core::sc_time_stamp().to_double()));
}

hdtlib::hl_lv_t<64> hif_verilog__system_time_hdtlib()
{
    return hdtlib::hl_lv_t<64>(static_cast<uint64_t>(sc_core::sc_time_stamp().to_double()));
}
#endif

} // namespace hif_verilog_standard
