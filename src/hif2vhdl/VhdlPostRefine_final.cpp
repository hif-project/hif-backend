/// @file VhdlPostRefine_final.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <algorithm>

#include "hif2vhdl/PostRefineMethods.hpp"
#include "hif2vhdl/PrintMethods.hpp"

using namespace hif;

namespace
{

// /////////////////////////////////////////////////////////////////////////////
// FinalRefineVisitor
// /////////////////////////////////////////////////////////////////////////////

/// @brief Visit the tree and generates scope-defined libraries, which are not
/// true libraries but the includes (DesignUnit, LibraryDef, etc) needed by printer.
/// Note: for LibraryDef, some includes are needed before defining new structures,
/// some other are the include of LibraryDef component. These last ones are marked
/// with a property.
/// It also fix the scope of objects (generally stored as 'instance' member of objects).
class FinalRefineVisitor : public GuideVisitor
{
public:
    typedef std::set<Scope *> ScopeSet;
    typedef std::map<std::string, ScopeSet> LibraryMap;
    typedef std::set<FieldReference *> FieldReferenceSet;

    FinalRefineVisitor(semantics::ILanguageSemantics *sem);
    ~FinalRefineVisitor();

    /// @name Scope-related visits.
    /// @{
    int visitContents(Contents &o);
    int visitExpression(Expression &o);
    int visitLibraryDef(LibraryDef &o);
    int visitSystem(System &o);
    int visitView(View &o);
    /// @}

    /// @name Symbol-related visits.
    /// @{
    int visitBit(Bit &o);
    int visitBitvector(Bitvector &o);
    int visitLibrary(Library &o);
    int visitSigned(Signed &o);
    int visitUnsigned(Unsigned &o);
    /// @}

    /// @name Special cases.
    /// @{

    /// @}

    /// @brief Generate all the needed includes.
    void doFixes();

private:
    // Disabled.
    FinalRefineVisitor(const FinalRefineVisitor &);
    FinalRefineVisitor &operator=(const FinalRefineVisitor &);

    semantics::ILanguageSemantics *_sem;
    HifFactory _factory;

    /// @brief Keep trace of current scope.
    Scope *_currentScope;

    LibraryMap _libraryMap;

    FieldReferenceSet _frSet;

    hif::Trash _trash;
};

FinalRefineVisitor::FinalRefineVisitor(semantics::ILanguageSemantics *sem)
    : _sem(sem)
    , _factory(sem)
    , _currentScope(nullptr)
    , _libraryMap()
    , _frSet()
    , _trash()
{
    // ntd
}

FinalRefineVisitor::~FinalRefineVisitor()
{
    // ntd
}

int FinalRefineVisitor::visitContents(Contents &o)
{
    //    Scope * restore = _currentScope;
    //    _currentScope = &o;
    GuideVisitor::visitContents(o);
    //    _currentScope = restore;

    return 0;
}

int FinalRefineVisitor::visitExpression(Expression &o)
{
    GuideVisitor::visitExpression(o);

    if (o.getOperator() != op_pow)
        return 0;
    Type *t = hif::semantics::getSemanticType(&o, _sem);
    messageAssert(t != nullptr, "Cannot type expression", &o, _sem);

    Real *r = dynamic_cast<Real *>(t);
    if (r == nullptr)
        return 0;

    _libraryMap["ieee_math_real"].insert(_currentScope);

    return 0;
}

int FinalRefineVisitor::visitLibraryDef(LibraryDef &o)
{
    if (o.isStandard())
        return 0;

    Scope *restore = _currentScope;
    _currentScope  = &o;
    GuideVisitor::visitLibraryDef(o);
    _currentScope = restore;

    return 0;
}

int FinalRefineVisitor::visitSystem(System &o)
{
    _currentScope = &o;
    GuideVisitor::visitSystem(o);

    return 0;
}

int FinalRefineVisitor::visitView(View &o)
{
    if (o.isStandard())
        return 0;

    Scope *restore = _currentScope;
    _currentScope  = &o;
    GuideVisitor::visitView(o);
    _currentScope = restore;

    return 0;
}

int FinalRefineVisitor::visitBit(Bit &o)
{
    GuideVisitor::visitBit(o);

    if (!o.isLogic())
        return 0;
    _libraryMap["ieee_std_logic_1164"].insert(_currentScope);

    return 0;
}

int FinalRefineVisitor::visitBitvector(Bitvector &o)
{
    GuideVisitor::visitBitvector(o);

    if (!o.isLogic())
        return 0;
    _libraryMap["ieee_std_logic_1164"].insert(_currentScope);

    return 0;
}

int FinalRefineVisitor::visitLibrary(Library &o)
{
    GuideVisitor::visitLibrary(o);

    Library *lib = &o;
    if (lib->isStandard() || !lib->isSystem())
        return 0;

    messageAssert(lib->getInstance() == nullptr, "Unexpected standard library (1)", &o, _sem);

    std::string libName = lib->getName();
    std::string libraryInclude;
    std::string package;
    if (libName.substr(0, 4) == "ieee") {
        libraryInclude = "ieee";
        package        = libName.substr(5); // skipping "ieee_"
    } else if (libName == "std_textio") {
        libraryInclude = "std";
        package        = "textio";
    } else if (libName == "standard") {
        libraryInclude = "";
        package        = "";
    } else {
        messageError("Unexpected standard library (2)", &o, _sem);
    }

    messageAssert(
        (libraryInclude.empty() && package.empty()) || (!libraryInclude.empty() && !package.empty()),
        "Unexpected standard library (3)", &o, _sem);

    if (libraryInclude.empty() && package.empty()) {
        // standard
        Instance *inst     = dynamic_cast<Instance *>(o.getParent());
        FieldReference *fr = nullptr;
        if (inst != nullptr)
            fr = dynamic_cast<FieldReference *>(inst->getParent());
        _trash.insert(&o);
        if (fr != nullptr) {
            _frSet.insert(fr);
        }
        return 0;
    }

    Library *l = _factory.library(libraryInclude.c_str(), nullptr, nullptr, false, true);
    lib->setName(package);
    lib->setInstance(l);

    return 0;
}

int FinalRefineVisitor::visitSigned(Signed &o)
{
    GuideVisitor::visitSigned(o);

    _libraryMap["ieee_numeric_std"].insert(_currentScope);

    return 0;
}

int FinalRefineVisitor::visitUnsigned(Unsigned &o)
{
    GuideVisitor::visitUnsigned(o);

    _libraryMap["ieee_numeric_std"].insert(_currentScope);

    return 0;
}

void FinalRefineVisitor::doFixes()
{
    for (LibraryMap::iterator i = _libraryMap.begin(); i != _libraryMap.end(); ++i) {
        const std::string &libName = i->first;
        for (ScopeSet::iterator j = i->second.begin(); j != i->second.end(); ++j) {
            Scope *s     = *j;
            Library *lib = _factory.library(libName.c_str(), nullptr, nullptr, false, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            const bool added                    = hif::manipulation::addUniqueObject(lib, s, addOpt);
            if (added) {
                lib->acceptVisitor(*this);
                lib->replace(nullptr);

                hif::manipulation::AddUniqueObjectOptions addOpt2;
                addOpt2.equalsOptions.checkOnlyNames = true;
                addOpt2.deleteIfNotAdded             = true;
                hif::manipulation::addUniqueObject(lib, s, addOpt2);
            }
        }
    }

    for (FieldReferenceSet::iterator i = _frSet.begin(); i != _frSet.end(); ++i) {
        FieldReference *fr = *i;
        Identifier *id     = new Identifier(fr->getName());
        fr->replace(id);
        delete fr;
    }

    _trash.clear();
}

} // namespace

void postRefinementsFinalStep(System *o, semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2VHDL", "postRefinementsFinalStep");

    FinalRefineVisitor irVis(sem);
    o->acceptVisitor(irVis);

    irVis.doFixes();

    hif::application_utils::restoreLogHeader();
}
