/// @file PreRefine_utilityLibraries.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/hif.hpp>

#include "hif2vhdl/PreRefineMethods.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

using namespace hif;
using namespace semantics;

namespace
{

// /////////////////////////////////////////////////////////////////////////////
// Refine introducing utility libraries
// /////////////////////////////////////////////////////////////////////////////

class PreRefine_utilityLibraries : public hif::GuideVisitor
{
public:
    typedef std::set<LibraryDef *> LibraryDefSet;

    /// @brief Default constructor and destructor.
    PreRefine_utilityLibraries(System *root, hif::semantics::ILanguageSemantics *sem);
    virtual ~PreRefine_utilityLibraries();

    virtual int visitContents(hif::Contents &o);
    virtual int visitLibraryDef(hif::LibraryDef &o);
    virtual int visitSigned(hif::Signed &o);
    virtual int visitSystem(hif::System &o);
    virtual int visitUnsigned(hif::Unsigned &o);
    virtual int visitView(hif::View &o);

    /// @brief Tells whether support libraries have been introduced.
    bool hasIntroducedLibraries();

    void addLibraryDefs();

private:
    // Disabled.
    PreRefine_utilityLibraries(const PreRefine_utilityLibraries &);
    PreRefine_utilityLibraries &operator=(const PreRefine_utilityLibraries &);

    /// @name Support creation common methods.
    //@{

    /// Adds the "hif_" + s library.
    void _addHifLibrary(const char *s);

    //@}

    /// Pointer to root, needed to add support standard libraries.
    System *_root;

    /// @brief The current scope (View) where library references will be added.
    Object *_scope;

    /// @brief Tells whether at least a library reference has been added.
    bool _introducedLibraries;

    LibraryDefSet _libraryDefSet;

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _factory;
};

PreRefine_utilityLibraries::PreRefine_utilityLibraries(System *root, hif::semantics::ILanguageSemantics *sem)
    : _root(root)
    , _scope(nullptr)
    , _introducedLibraries(false)
    , _libraryDefSet()
    , _sem(sem)
    , _factory(sem)
{
    // ntd
}

PreRefine_utilityLibraries::~PreRefine_utilityLibraries()
{
    // ntd
}

bool PreRefine_utilityLibraries::hasIntroducedLibraries() { return _introducedLibraries; }

void PreRefine_utilityLibraries::addLibraryDefs()
{
    for (LibraryDefSet::iterator i = _libraryDefSet.begin(); i != _libraryDefSet.end(); ++i) {
        hif::manipulation::addUniqueObject(*i, _root->libraryDefs);
    }
}

int PreRefine_utilityLibraries::visitContents(hif::Contents &o)
{
    //Object * restore = _scope;
    //_scope = &o;
    GuideVisitor::visitContents(o);
    //_scope = restore;
    return 0;
}

int PreRefine_utilityLibraries::visitLibraryDef(hif::LibraryDef &o)
{
    Object *restore = _scope;
    _scope          = &o;
    GuideVisitor::visitLibraryDef(o);
    _scope = restore;
    return 0;
}

int PreRefine_utilityLibraries::visitSigned(Signed &o)
{
    GuideVisitor::visitSigned(o);
    _libraryDefSet.insert(_sem->getStandardLibrary("hif_vhdl_ieee_numeric_std"));
    return 0;
}

int PreRefine_utilityLibraries::visitSystem(hif::System &o)
{
    Object *restore = _scope;
    _scope          = &o;
    GuideVisitor::visitSystem(o);
    _scope = restore;
    return 0;
}

int PreRefine_utilityLibraries::visitUnsigned(Unsigned &o)
{
    GuideVisitor::visitUnsigned(o);
    _libraryDefSet.insert(_sem->getStandardLibrary("hif_vhdl_ieee_numeric_std"));
    return 0;
}

int PreRefine_utilityLibraries::visitView(hif::View &o)
{
    Object *restore = _scope;
    _scope          = &o;
    GuideVisitor::visitView(o);
    _scope = restore;
    return 0;
}

void PreRefine_utilityLibraries::_addHifLibrary(const char *c)
{
    std::string nn("hif_");
    nn += c;

    _introducedLibraries |= hif::backends::addHifLibrary(nn, _scope, _root, _sem);
}

} // namespace

bool fixUtilityLibraries(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2VHDL", "fixUtilityLibraries");

    PreRefine_utilityLibraries vis(o, sem);
    o->acceptVisitor(vis);

    vis.addLibraryDefs();

    hif::application_utils::restoreLogHeader();

    return vis.hasIntroducedLibraries();
}
