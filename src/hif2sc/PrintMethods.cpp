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
    ~CollectConstTemplatesVisitor() override;

    auto visitView(View &o) -> int override;
    auto visitViewReference(ViewReference &o) -> int override;

private:
    CollectConstTemplatesVisitor(const CollectConstTemplatesVisitor &)                     = delete;
    auto operator=(const CollectConstTemplatesVisitor &) -> CollectConstTemplatesVisitor & = delete;

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

auto CollectConstTemplatesVisitor::visitView(View &o) -> int
{
    GuideVisitor::visitView(o);

    for (BList<Declaration>::iterator i = o.templateParameters.begin(); i != o.templateParameters.end();) {
        auto *vtp = dynamic_cast<ValueTP *>(*i);
        if (vtp == nullptr || vtp->isCompileTimeConstant()) {
            ++i;
            continue;
        }

        _ctmList[&o].push_back(vtp);
        i = i.remove();
    }

    return 0;
}

auto CollectConstTemplatesVisitor::visitViewReference(ViewReference &o) -> int
{
    GuideVisitor::visitViewReference(o);

    for (BList<TPAssign>::iterator i = o.templateParameterAssigns.begin(); i != o.templateParameterAssigns.end();) {
        auto *vtpa = dynamic_cast<ValueTPAssign *>(*i);
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
    ~FindTemplateVisitor_t() override;

    void clean();
    auto TPregardless() const -> bool;
    auto ownTemplate() const -> bool;
    auto ownTemplateOnly() const -> bool;

    auto visitFunction(Function &o) -> int override;
    auto visitLibraryDef(LibraryDef &o) -> int override;
    auto visitProcedure(Procedure &o) -> int override;
    auto visitSystem(System &o) -> int override;
    auto visitTypeDef(TypeDef &o) -> int override;
    auto visitView(View &o) -> int override;

private:
    FindTemplateVisitor_t(const FindTemplateVisitor_t &other)                     = delete;
    auto operator=(const FindTemplateVisitor_t &other) -> FindTemplateVisitor_t & = delete;

    // If set, result concerns the object subtree only (root is not included).
    // If not set, result concerns the root object only.
    bool _subTreeOnly;

    // Indicates whether the subtree contains objects that own TP.
    bool _ownTemplate{false};

    // Indicates whether the subtree contains only objects that own TP.
    // Note: to be checked only after _ownTemplate.
    bool _ownTemplateOnly{true};
};

FindTemplateVisitor_t::FindTemplateVisitor_t(bool subTreeOnly)
    : _subTreeOnly(subTreeOnly)

{
}

FindTemplateVisitor_t::~FindTemplateVisitor_t() = default;

void FindTemplateVisitor_t::clean()
{
    _ownTemplate     = false;
    _ownTemplateOnly = true;
}

auto FindTemplateVisitor_t::TPregardless() const -> bool { return !_ownTemplate; }

auto FindTemplateVisitor_t::ownTemplate() const -> bool { return _ownTemplate; }

auto FindTemplateVisitor_t::ownTemplateOnly() const -> bool
{
    // Note: _ownTemplateOnly is init to true until it is falsified. Here
    // _ownTemplate acts as a safety check.
    return _ownTemplate && _ownTemplateOnly;
}

auto FindTemplateVisitor_t::visitFunction(Function &o) -> int
{
    messageDebugAssert(!_subTreeOnly, "Unexpected case", &o, nullptr); // leaf

    if (!o.templateParameters.empty() || o.checkProperty(PROPERTY_CONSTEXPR)) {
        _ownTemplate = true;
    } else {
        _ownTemplateOnly = false;
    }
    return 0;
}

auto FindTemplateVisitor_t::visitLibraryDef(LibraryDef &o) -> int
{
    if (!_subTreeOnly) {
        return 0;
    }

    // Considering System as entry point, we are only interested in its declarations.
    _subTreeOnly = false;
    for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
        (*it)->acceptVisitor(*this);
    }
    return 0;
}

auto FindTemplateVisitor_t::visitProcedure(Procedure &o) -> int
{
    messageDebugAssert(!_subTreeOnly, "Unexpected case", &o, nullptr); // leaf

    if (!o.templateParameters.empty() || o.checkProperty(PROPERTY_CONSTEXPR)) {
        _ownTemplate = true;
    } else {
        _ownTemplateOnly = false;
    }
    return 0;
}

auto FindTemplateVisitor_t::visitSystem(System &o) -> int
{
    if (!_subTreeOnly) {
        return 0;
    }

    // Considering System as entry point, we are only interested in its declarations.
    _subTreeOnly = false;
    for (BList<Declaration>::iterator it = o.declarations.begin(); it != o.declarations.end(); ++it) {
        (*it)->acceptVisitor(*this);
    }
    return 0;
}

auto FindTemplateVisitor_t::visitTypeDef(TypeDef &o) -> int
{
    assert(!_subTreeOnly); // leaf

    if (!o.templateParameters.empty()) {
        _ownTemplate = true;
    } else {
        _ownTemplateOnly = false;
    }
    return 0;
}

auto FindTemplateVisitor_t::visitView(View &o) -> int
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

auto ownTemplate(Object *obj, bool subTreeOnly) -> bool
{
    FindTemplateVisitor_t vis(subTreeOnly);
    obj->acceptVisitor(vis);
    return vis.ownTemplate();
}

auto ownTemplateOnly(Object *obj, bool subTreeOnly) -> bool
{
    FindTemplateVisitor_t vis(subTreeOnly);
    obj->acceptVisitor(vis);
    return vis.ownTemplateOnly();
}

auto checkLanguage(Object *obj, LanguageID language) -> bool
{
    View *view = hif::getNearestParent<View>(obj, true);
    if (view != nullptr && view->getLanguageID() == language) {
        return true;
    }

    auto *libD = hif::getNearestParent<LibraryDef>(obj, true);
    if (libD != nullptr && libD->getLanguageID() == language) {
        return true;
    }

    auto *sys = hif::getNearestParent<System>(obj, true);
    return sys != nullptr && sys->getLanguageID() == language;
}

auto getLanguage(LanguageID language) -> std::string
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
