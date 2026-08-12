/// @file PrintMethods.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <cstdlib>
#include <fstream>

#include <hif/hif.hpp>

#include "hif2sc/PrintSystemCVisitor.hpp"
#include "hif2sc/globals.hpp"
#include "hif2sc/hif2scParseLine.hpp"

/// @name Printing-related methods.
/// @{

void collectConstTemplates(
    hif::System *o,
    PrintSystemCVisitor::ConstTemplateMap &ctmList,
    hif::semantics::ILanguageSemantics *sem);

/// @brief Generate header file(s).
void printHeaders(hif::System *sys, hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList);

/// @brief Generate implementation file(s).
void printImplementations(hif::System *sys, hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList);

/// @}

/// @name Language-related methods.
/// @{

/// @brief Check if the implementation language is the one desired.
auto checkLanguage(hif::Object *obj, hif::LanguageID lang) -> bool;

/// @brief Get the string correspondent to the implementation language.
auto getLanguage(hif::LanguageID lang) -> std::string;

/// @}

/// @name Template-parameters-related methods.
/// @{

/// @brief: Check if the passed object owns template parameters or its contents
/// contain template parameters.
/// @param obj The object to check.
/// @param subTreeOnly Indicates whether check subtree too.
auto ownTemplate(hif::Object *obj, bool subTreeOnly) -> bool;

/// @brief: Check if the passed object owns only template parameters or its
/// contents contain only template parameters.
/// @param obj The object to check.
/// @param subTreeOnly Indicates whether check subtree too.
auto ownTemplateOnly(hif::Object *obj, bool subTreeOnly) -> bool;

/// @}
