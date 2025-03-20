/// @file hif_verilog_standard.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <systemc>

#include "hif2scSupport/hif2scSupport/config.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

namespace hif_verilog_standard
{

template <int times, int size>
sc_dt::sc_lv<times * size> hif_verilog_iterated_concat(sc_dt::sc_lv<size> expression);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
template <int times, int size>
hdtlib::hl_lv_t<times * size> hif_verilog_iterated_concat(hdtlib::hl_lv_t<size> expression);
#endif

// ///////////////////////////////////////////////////////////////////
// System tasks (Clause 17)
// ///////////////////////////////////////////////////////////////////

#define hif_verilog__system_finish(P)                                                                                  \
    internal::hif_verilog__system_finish_impl(                                                                         \
        __FILE__, __LINE__, __func__, hif_verilog_standard::internal::hif_verilog__system_getParam(P));
#define hif_verilog__system_stop(P)                                                                                    \
    internal::hif_verilog__system_stop_impl(                                                                           \
        __FILE__, __LINE__, __func__, hif_verilog_standard::internal::hif_verilog__system_getParam(P));

#ifdef HIF2SCSUPPORT_USE_HDTLIB
#define hif_verilog__system_finish_hdtlib(P)                                                                           \
    internal::hif_verilog__system_finish_impl(                                                                         \
        __FILE__, __LINE__, __func__, hif_verilog_standard::internal::hif_verilog__system_getParam_hdtlib(P));
#define hif_verilog__system_stop_hdtlib(P)                                                                             \
    internal::hif_verilog__system_stop_impl(                                                                           \
        __FILE__, __LINE__, __func__, hif_verilog_standard::internal::hif_verilog__system_getParam_hdtlib(P));
#endif

namespace internal
{

HIF2SCSUPPORT_EXPORT
sc_dt::sc_lv<32> hif_verilog__system_getParam(sc_dt::sc_lv<32> p = 1);

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_finish_impl(const char *f, const int l, const char *func, sc_dt::sc_lv<32> param1 = 1);

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_stop_impl(const char *f, const int l, const char *func, sc_dt::sc_lv<32> param1 = 1);

#ifdef HIF2SCSUPPORT_USE_HDTLIB

HIF2SCSUPPORT_EXPORT
hdtlib::hl_lv_t<32> hif_verilog__system_getParam_hdtlib(hdtlib::hl_lv_t<32> p = 1);

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_finish_impl(const char *f, const int l, const char *func, hdtlib::hl_lv_t<32> param1 = 1);
HIF2SCSUPPORT_EXPORT
void hif_verilog__system_stop_impl(const char *f, const int l, const char *func, hdtlib::hl_lv_t<32> param1 = 1);

#endif

} // namespace internal

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_fdisplay(const int fd, const char *const s, ...);

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_fscanf(const int fd, const char *const s, ...);

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_fwrite(const int fd, const char *const s, ...);

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_fflush(const int fd);

HIF2SCSUPPORT_EXPORT
void hif_verilog__system_fclose(const int fd);

HIF2SCSUPPORT_EXPORT
int hif_verilog__system_feof(const int fd);

HIF2SCSUPPORT_EXPORT
sc_dt::sc_lv<32> hif_verilog__system_stime();
HIF2SCSUPPORT_EXPORT
sc_dt::sc_lv<64> hif_verilog__system_time();
HIF2SCSUPPORT_EXPORT
double hif_verilog__system_realtime();
HIF2SCSUPPORT_EXPORT
int32_t hif_verilog__system_random(const uint64_t seed = uint64_t(-1));

template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    sc_dt::sc_bv<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    sc_dt::sc_lv<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint8_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint16_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint32_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    uint64_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int8_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int16_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int32_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    int64_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    bool (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);

template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    sc_dt::sc_bv<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    sc_dt::sc_lv<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint8_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint16_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint32_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    uint64_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int8_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int16_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int32_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);
template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    int64_t (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);

template <size_t L>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    bool (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1),
    const int min        = 0,
    const int direction  = 1);

#ifdef HIF2SCSUPPORT_USE_HDTLIB
HIF2SCSUPPORT_EXPORT
hdtlib::hl_lv_t<32> hif_verilog__system_stime_hdtlib();

HIF2SCSUPPORT_EXPORT
hdtlib::hl_lv_t<64> hif_verilog__system_time_hdtlib();

template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    hdtlib::hl_bv_t<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1));

template <size_t L, int W>
void hif_verilog__system_readmemb(
    const std::string &file_name,
    hdtlib::hl_lv_t<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1));

template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    hdtlib::hl_bv_t<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1));

template <size_t L, int W>
void hif_verilog__system_readmemh(
    const std::string &file_name,
    hdtlib::hl_lv_t<W> (&memory_name)[L],
    const int start_addr = 0,
    const int stop_addr  = int(L - 1));

#endif

} // namespace hif_verilog_standard

#include "hif2scSupport/hif_verilog_standard.i.hpp"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
