/// @file globals.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

/////////////////////////////////////////////////////////////////
// properties used by fix description.
/////////////////////////////////////////////////////////////////
extern const char *PROPERTY_DO_NOT_ADD_READ;
extern const char *PROPERTY_RISING_EDGE;
extern const char *PROPERTY_IMPLEMENTATION_INCLUDE;
extern const char *PROPERTY_TYPDEF_DESIGN_UNIT;

/////////////////////////////////////////////////////////////////
// headers
/////////////////////////////////////////////////////////////////

extern const char *HEADER_MATH;

/////////////////////////////////////////////////////////////////
// functions
/////////////////////////////////////////////////////////////////

/// @brief Manage given identifier: if its type is different from string,
/// function add a cast to its declaration type, otherwise add string enum
/// and add a new enum value with identifier name to enum.
/// @param o The identifier to manage.
///
void manageIdentifierTP(hif::Identifier &o);

/// @brief Add the global string enum used to replace original template
/// parameter of string type.
/// @param o An object related to the tree.
///
auto addAndGetStringEnum(hif::Object *o) -> hif::Enum *;

/// @brief Add a value to global string enum.
/// @param o An object related to the tree.
/// @param v The Value TP that has the value to set.
void addStringEnumValue(hif::Object *o, hif::ValueTP *v);

/// @brief Add a value to global string enum of given name.
/// @param o An object related to the tree.
/// @param enum_name The name of enum value to add.
void addStringEnumEntry(hif::Object *o, const std::string &enum_name);

/// @brief Print error messages with some other utils information
/// @param msg The message to print.
/// @param obj The optional object that causes the error.
/// @param type The optional type that causes the error.
void error(const char *msg, hif::Object *obj = nullptr, hif::Type *type = nullptr);
