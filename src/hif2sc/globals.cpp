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

using namespace hif;

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
void manageIdentifierTP(Identifier &o)
{
    // the semantics is Hif because this function is used before standardization.
    DataDeclaration *decl = hif::semantics::getDeclaration(&o, hif::semantics::HIFSemantics::getInstance());

    // type is not string: add cast to declaration type.
    String *str = dynamic_cast<String *>(decl->getType());
    if (str == nullptr) {
        Cast *c = new Cast();
        o.replace(c);
        c->setValue(&o);
        c->setType(hif::copy(decl->getType()));

        return;
    }

    // type is a string: get the other text operator.
    Expression *exp = dynamic_cast<Expression *>(o.getParent());
    // Could be the right hand side of an assignment.
    if (exp == nullptr)
        return;

    StringValue *text = nullptr;
    if (exp->getValue1() == &o) {
        text = dynamic_cast<StringValue *>(exp->getValue2());
    } else {
        text = dynamic_cast<StringValue *>(exp->getValue1());
    }
    messageAssert(text != nullptr, "Unexpected case", exp, nullptr);

    // replace text with an identifier with name of text.
    Identifier *id = new Identifier();
    id->setName(text->getValue());
    text->replace(id);

    // create and add the new enum value.
    addStringEnumEntry(&o, id->getName());

    delete text;
}

Enum *addAndGetStringEnum(Object *o)
{
    System *sys = hif::getNearestParent<System>(o);
    messageAssert(sys != nullptr, "System not found", o, nullptr);

    // add enum if not already exists.
    TypeDef *td = new TypeDef();
    td->setName(NameTable::getInstance()->hifStringNames());
    td->setType(new Enum());
    td->setOpaque(true);
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    bool ins                            = hif::manipulation::addUniqueObject(td, sys->declarations, addOpt);

    // if not already present return the enum
    if (ins)
        return static_cast<Enum *>(td->getType());

    // otherwise get and return the enum
    delete td;
    TypeReference *tr = new TypeReference();
    tr->setName(NameTable::getInstance()->hifStringNames());
    hif::semantics::DeclarationOptions dopt;
    dopt.location = o;
    TypeDef *tde =
        dynamic_cast<TypeDef *>(hif::semantics::getDeclaration(tr, hif::semantics::HIFSemantics::getInstance(), dopt));
    messageAssert(tde != nullptr, "Unexpected declaration", tr, nullptr);
    Enum *e = dynamic_cast<Enum *>(tde->getType());
    messageDebugAssert(e != nullptr, "Unexpected case", tde->getType(), nullptr);
    return e;
}

void addStringEnumEntry(Object *o, const std::string &enum_name)
{
    // get the global string enum.
    Enum *e = addAndGetStringEnum(o);

    // create the enum value
    TypeReference *tr = new TypeReference();
    tr->setName(NameTable::getInstance()->hifStringNames());

    EnumValue *ev = new EnumValue();
    ev->setName(enum_name);
    ev->setType(tr);

    // adding new value
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(ev, e->values, addOpt);
}

void addStringEnumValue(Object *o, ValueTP *vtp)
{
    StringValue *text = dynamic_cast<StringValue *>(vtp->getValue());
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
    Identifier *id = new Identifier();
    id->setName(text->getValue());
    delete vtp->setValue(id);
}
