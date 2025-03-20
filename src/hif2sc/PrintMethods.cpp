/// @file PrintMethods.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2sc/PrintMethods.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

using std::ofstream;
using std::string;
using namespace hif;

namespace
{

// ///////////////////////////////////////////////////////////////////
// CollectConstTemplatesVisitor
// ///////////////////////////////////////////////////////////////////
class CollectConstTemplatesVisitor : public GuideVisitor
{
public:
    CollectConstTemplatesVisitor(
        PrintSystemCVisitor::ConstTemplateMap &ctmList,
        hif::semantics::ILanguageSemantics *sem);
    ~CollectConstTemplatesVisitor();

    int visitView(View &o);
    int visitViewReference(ViewReference &o);

private:
    CollectConstTemplatesVisitor(const CollectConstTemplatesVisitor &);
    CollectConstTemplatesVisitor &operator=(const CollectConstTemplatesVisitor &);

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _f;

    /// @brief The map of constant template.
    PrintSystemCVisitor::ConstTemplateMap &_ctmList;
};

CollectConstTemplatesVisitor::CollectConstTemplatesVisitor(
    PrintSystemCVisitor::ConstTemplateMap &ctmList,
    hif::semantics::ILanguageSemantics *sem)
    : _sem(sem)
    , _f(sem)
    , _ctmList(ctmList)
{
    // ntd
}

CollectConstTemplatesVisitor::~CollectConstTemplatesVisitor()
{
    // ntd
}

int CollectConstTemplatesVisitor::visitView(View &o)
{
    GuideVisitor::visitView(o);

    for (BList<Declaration>::iterator i = o.templateParameters.begin(); i != o.templateParameters.end();) {
        ValueTP *vtp = dynamic_cast<ValueTP *>(*i);
        if (vtp == nullptr || vtp->isCompileTimeConstant()) {
            ++i;
            continue;
        }

        _ctmList[&o].push_back(vtp);
        i = i.remove();
    }

    return 0;
}

int CollectConstTemplatesVisitor::visitViewReference(ViewReference &o)
{
    GuideVisitor::visitViewReference(o);

    for (BList<TPAssign>::iterator i = o.templateParameterAssigns.begin(); i != o.templateParameterAssigns.end();) {
        ValueTPAssign *vtpa = dynamic_cast<ValueTPAssign *>(*i);
        if (vtpa == nullptr) {
            ++i;
            continue;
        }

        ValueTP *vtp = hif::semantics::getDeclaration(vtpa, _sem);
        messageAssert(vtp != nullptr, "Declaration not found", vtpa, _sem);

        if (vtp->isCompileTimeConstant()) {
            ++i;
            continue;
        }

        _ctmList[&o].push_back(vtpa);
        i = i.remove();
    }

    return 0;
}

// ///////////////////////////////////////////////////////////////////
// FindTemplateVisitor
// ///////////////////////////////////////////////////////////////////

/// @brief This visitor is used to understand if inner components of analyzed
/// DesignUnit (or LibraryDef) own template parameters. For LibraryDef this
/// is useful to split the print inside .hpp, .i.hpp, .cc files.
class FindTemplateVisitor_t : public GuideVisitor
{
public:
    explicit FindTemplateVisitor_t(bool subTreeOnly);
    ~FindTemplateVisitor_t();

    void clean();
    bool TPregardless();
    bool ownTemplate();
    bool ownTemplateOnly();

    virtual int visitFunction(Function &o);
    virtual int visitLibraryDef(LibraryDef &o);
    virtual int visitProcedure(Procedure &o);
    virtual int visitSystem(System &o);
    virtual int visitTypeDef(TypeDef &o);
    virtual int visitView(View &o);

private:
    FindTemplateVisitor_t(const FindTemplateVisitor_t &other);
    FindTemplateVisitor_t &operator=(const FindTemplateVisitor_t &other);

    // If set, result concerns the object subtree only (root is not included).
    // If not set, result concerns the root object only.
    bool _subTreeOnly;

    // Indicates whether the subtree contains objects that own TP.
    bool _ownTemplate;

    // Indicates whether the subtree contains only objects that own TP.
    // Note: to be checked only after _ownTemplate.
    bool _ownTemplateOnly;
};

FindTemplateVisitor_t::FindTemplateVisitor_t(bool subTreeOnly)
    : _subTreeOnly(subTreeOnly)
    , _ownTemplate(false)
    , _ownTemplateOnly(true)
{
}

FindTemplateVisitor_t::~FindTemplateVisitor_t() {}

void FindTemplateVisitor_t::clean()
{
    _ownTemplate     = false;
    _ownTemplateOnly = true;
}

bool FindTemplateVisitor_t::TPregardless() { return _ownTemplate == false; }

bool FindTemplateVisitor_t::ownTemplate() { return _ownTemplate; }

bool FindTemplateVisitor_t::ownTemplateOnly()
{
    // Note: _ownTemplateOnly is init to true until it is falsified. Here
    // _ownTemplate acts as a safety check.
    return _ownTemplate && _ownTemplateOnly;
}

int FindTemplateVisitor_t::visitFunction(Function &o)
{
    messageDebugAssert(!_subTreeOnly, "Unexpected case", &o, nullptr); // leaf

    if (!o.templateParameters.empty() || o.checkProperty(PROPERTY_CONSTEXPR)) {
        _ownTemplate = true;
    } else {
        _ownTemplateOnly = false;
    }
    return 0;
}

int FindTemplateVisitor_t::visitLibraryDef(LibraryDef &o)
{
    if (!_subTreeOnly)
        return 0;

    // Considering System as entry point, we are only interested in its declarations.
    _subTreeOnly = false;
    for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
        (*it)->acceptVisitor(*this);
    }
    return 0;
}

int FindTemplateVisitor_t::visitProcedure(Procedure &o)
{
    messageDebugAssert(!_subTreeOnly, "Unexpected case", &o, nullptr); // leaf

    if (!o.templateParameters.empty() || o.checkProperty(PROPERTY_CONSTEXPR)) {
        _ownTemplate = true;
    } else {
        _ownTemplateOnly = false;
    }
    return 0;
}

int FindTemplateVisitor_t::visitSystem(System &o)
{
    if (!_subTreeOnly)
        return 0;

    // Considering System as entry point, we are only interested in its declarations.
    _subTreeOnly = false;
    for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
        (*it)->acceptVisitor(*this);
    }
    return 0;
}

int FindTemplateVisitor_t::visitTypeDef(TypeDef &o)
{
    assert(!_subTreeOnly); // leaf

    if (!o.templateParameters.empty()) {
        _ownTemplate = true;
    } else {
        _ownTemplateOnly = false;
    }
    return 0;
}

int FindTemplateVisitor_t::visitView(View &o)
{
    if (_subTreeOnly) {
        _subTreeOnly = false;
        GuideVisitor::visitView(o);
        return 0;
    }

    if (!o.templateParameters.empty()) {
        _ownTemplate = true;
    } else {
        _ownTemplateOnly = false;
    }

    return 0;
}

} // namespace

void collectConstTemplates(
    hif::System *o,
    PrintSystemCVisitor::ConstTemplateMap &ctmList,
    hif::semantics::ILanguageSemantics *sem)
{
    hif::semantics::updateDeclarations(o, sem);

    CollectConstTemplatesVisitor v(ctmList, sem);
    o->acceptVisitor(v);
}

bool ownTemplate(Object *obj, bool subTreeOnly)
{
    FindTemplateVisitor_t vis(subTreeOnly);
    obj->acceptVisitor(vis);
    return vis.ownTemplate();
}

bool ownTemplateOnly(Object *obj, bool subTreeOnly)
{
    FindTemplateVisitor_t vis(subTreeOnly);
    obj->acceptVisitor(vis);
    return vis.ownTemplateOnly();
}

bool checkLanguage(Object *obj, LanguageID language)
{
    View *view = hif::getNearestParent<View>(obj, true);
    if (view != nullptr && view->getLanguageID() == language)
        return true;

    LibraryDef *libD = hif::getNearestParent<LibraryDef>(obj, true);
    if (libD != nullptr && libD->getLanguageID() == language)
        return true;

    System *sys = hif::getNearestParent<System>(obj, true);
    if (sys != nullptr && sys->getLanguageID() == language)
        return true;

    return false;
}

std::string getLanguage(LanguageID language)
{
    switch (language) {
    case hif::rtl:
        return string("SystemC");
    case hif::tlm:
        return string("TLM2.0 SystemC");
    case hif::cpp:
        return string("C++");
    case hif::c:
        return string("C");
    case hif::psl:
        return string("PSL");
    case hif::ams:
        return string("AMS");
    default:
        break;
    }

    messageError("Unsupported implementation language!\n", nullptr, nullptr);
}
