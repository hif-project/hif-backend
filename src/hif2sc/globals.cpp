/// @file globals.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cstdlib>
#include <iostream>

#include <hif/hif.hpp>

#include "hif2sc/globals.hpp"

// ///////////////////////////////////////////////////////////////////
// Global initialization
// ///////////////////////////////////////////////////////////////////
const char *PROPERTY_DO_NOT_ADD_READ        = "do_not_add_read";
const char *PROPERTY_RISING_EDGE            = "rising_edge_support_signal";
const char *PROPERTY_IMPLEMENTATION_INCLUDE = "implementation_include";
const char *PROPERTY_TYPDEF_DESIGN_UNIT     = "typdef_design_unit";

/////////////////////////////////////////////////////////////////
// headers
/////////////////////////////////////////////////////////////////

const char *HEADER_MATH = "hif2sc_math";

// ///////////////////////////////////////////////////////////////////
// Global functions:
// ///////////////////////////////////////////////////////////////////
void manageIdentifierTP(hif::Identifier &o)
{
    // the semantics is Hif because this function is used before standardization.
    auto decl = hif::semantics::getDeclaration(&o, hif::semantics::HIFSemantics::getInstance());

    // type is not string: add cast to declaration type.
    auto str = dynamic_cast<hif::String *>(decl->getType());
    if (str == nullptr) {
        auto c = new hif::Cast();
        o.replace(c);
        c->setValue(&o);
        c->setType(hif::copy(decl->getType()));
        return;
    }

    // type is a string: get the other text operator.
    auto exp = dynamic_cast<hif::Expression *>(o.getParent());
    // Could be the right hand side of an assignment.
    if (exp == nullptr) {
        return;
    }

    hif::StringValue *text = nullptr;
    if (exp->getValue1() == &o) {
        text = dynamic_cast<hif::StringValue *>(exp->getValue2());
    } else {
        text = dynamic_cast<hif::StringValue *>(exp->getValue1());
    }
    messageAssert(text != nullptr, "Unexpected case", exp, nullptr);

    // replace text with an identifier with name of text.
    auto id = new hif::Identifier();
    id->setName(text->getValue());
    text->replace(id);

    // create and add the new enum value.
    addStringEnumEntry(&o, id->getName());

    delete text;
}

auto addAndGetStringEnum(hif::Object *o) -> hif::Enum *
{
    auto sys = hif::getNearestParent<hif::System>(o);
    messageAssert(sys != nullptr, "System not found", o, nullptr);

    // add enum if not already exists.
    auto td = new hif::TypeDef();
    td->setName(hif::NameTable::getInstance()->hifStringNames());
    td->setType(new hif::Enum());
    td->setOpaque(true);
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    bool ins                            = hif::manipulation::addUniqueObject(td, sys->declarations, addOpt);

    // if not already present return the enum
    if (ins) {
        return dynamic_cast<hif::Enum *>(td->getType());
    }

    // Delete the type definition if not added.
    delete td;
    
    // Prepare the declaration options.
    hif::semantics::DeclarationOptions dopt;
    dopt.location = o;

    // Build the type reference.
    auto tr = new hif::TypeReference();
    tr->setName(hif::NameTable::getInstance()->hifStringNames());
    // Get the semantic instance.
    auto sem = hif::semantics::HIFSemantics::getInstance();
    // Get the declaration.
    auto decl = hif::semantics::getDeclaration(tr, sem, dopt);
    // Cast the declaration to a type definition.
    auto tde = dynamic_cast<hif::TypeDef *>(decl);
    messageAssert(tde != nullptr, "Unexpected declaration", tr, nullptr);
    auto e = dynamic_cast<hif::Enum *>(tde->getType());
    messageDebugAssert(e != nullptr, "Unexpected case", tde->getType(), nullptr);
    return e;
}

void addStringEnumEntry(hif::Object *o, const std::string &enum_name)
{
    // get the global string enum.
    auto e = addAndGetStringEnum(o);

    // create the enum value
    auto tr = new hif::TypeReference();
    tr->setName(hif::NameTable::getInstance()->hifStringNames());

    auto ev = new hif::EnumValue();
    ev->setName(enum_name);
    ev->setType(tr);

    // adding new value
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(ev, e->values, addOpt);
}

void addStringEnumValue(hif::Object *o, hif::ValueTP *vtp)
{
    auto text = dynamic_cast<hif::StringValue *>(vtp->getValue());
    if (text == nullptr) {
        // We support only string constants for the moment:
        messageDebugAssert(vtp->getValue() == nullptr, "Unexpected init val", vtp, nullptr);
        delete vtp->setValue(nullptr);
        // Ensuring the existence of global enum:
        addAndGetStringEnum(o);
        return;
    }

    // create the enum value
    addStringEnumEntry(o, text->getValue());

    // change initial value to the new enum constant.
    auto id = new hif::Identifier();
    id->setName(text->getValue());
    delete vtp->setValue(id);
}
