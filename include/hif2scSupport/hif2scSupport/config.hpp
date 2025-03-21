/// @file config.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#if (defined _MSC_VER)

#if (defined COMPILE_HIF2SCSUPPORT_LIB)
// Compiling dynamic
#define HIF2SCSUPPORT_DEVICE
#define HIF2SCSUPPORT_EXPORT __declspec(dllexport)
#define HIF2SCSUPPORT_STL_EXTERN
#define HIF2SCSUPPORT_STL_EXPORT __declspec(dllexport)
#elif (defined USE_HIF2SCSUPPORT_LIB)
// Linking dynamic
#define HIF2SCSUPPORT_DEVICE
#define HIF2SCSUPPORT_EXPORT     __declspec(dllimport)
#define HIF2SCSUPPORT_STL_EXTERN extern
#define HIF2SCSUPPORT_STL_EXPORT __declspec(dllimport)
#else
// Compiling/linking static
#define HIF2SCSUPPORT_DEVICE
#define HIF2SCSUPPORT_EXPORT
#define HIF2SCSUPPORT_STL_EXTERN
#define HIF2SCSUPPORT_STL_EXPORT
#endif

// dll-interface export problem
#pragma warning(disable : 4251)

#else // LINUX
#define HIF2SCSUPPORT_DEVICE
#define HIF2SCSUPPORT_EXPORT __attribute__((visibility("default")))
#endif

#if (defined _MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127)
#pragma warning(disable : 4512)
#endif

#ifdef HIF2SCSUPPORT_USE_HDTLIB
#include <hdtlib/hdtlib.hh>
#endif
//#ifdef HIF_USE_SYSTEMC
#include <cstdint>
#include <systemc>
//#endif

#if (defined _MSC_VER)
#pragma warning(pop)
#endif

#if __cplusplus >= 201103L
#define HIF_CONSTEXPR constexpr
#else
#define HIF_CONSTEXPR
#endif
