/// @file PostRefine_cpp98.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cstdlib>
#include <iostream>

#include "hif2sc/PostRefineMethods.hpp"
#include "hif2sc/globals.hpp"

namespace
{

// ///////////////////////////////////////////////////////////////////
// CppStandardRefineVisitor
// ///////////////////////////////////////////////////////////////////

/// This visitor is intended to grant retro-compatibility of generated code if
/// requested by the user.
/// By default, generated code exploits features of standard ISO/IEC 14882:2011 (C++11).
/// This visitor modifies Hif tree to match features of standard
/// ISO/IEC 14882:1998 (C++98).

class CppStandardRefineVisitor : public hif::GuideVisitor
{
public:
    using SubProgramSet = std::set<hif::SubProgram *>;

    /// @brief Default constructor.
    CppStandardRefineVisitor(
        hif::System *system,
        hif::semantics::ILanguageSemantics *sem,
        hif::semantics::ILanguageSemantics *checkSem);
    /// @brief Destructor.
    ~CppStandardRefineVisitor() override;

    /// @brief Remove template parameters from TypeDef, putting it inside a new
    /// templated DesignUnit.
    auto visitTypeDef(hif::TypeDef &o) -> int override;

    /// @brief Inlines functions which are marked as constexpr.
    auto visitFunctionCall(hif::FunctionCall &o) -> int override;
    auto visitProcedureCall(hif::ProcedureCall &o) -> int override;

    /// @name Collects methods in set to remove constexpr property
    /// and collets in other set to remove default values template parameter.
    /// @{

    auto visitFunction(hif::Function &o) -> int override;
    auto visitProcedure(hif::Procedure &o) -> int override;

    /// @}

    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitView(hif::View &o) -> int override;

private:
    hif::HifFactory _factory;
    hif::semantics::ILanguageSemantics *_sem;
    hif::semantics::ILanguageSemantics *_checkSem;
    SubProgramSet _fixedSet;
    SubProgramSet _recursionSet;
    bool _needToReset{false};
    hif::System *_system;
    SubProgramSet _methodsWithTPDefaultValue;

    CppStandardRefineVisitor(const CppStandardRefineVisitor &)                     = delete;
    auto operator=(const CppStandardRefineVisitor &) -> CppStandardRefineVisitor & = delete;

    /// @brief Update the references.
    auto
    _updateReferences(hif::View *newDecl, const std::string &duName, hif::semantics::ReferencesSet &references) -> int;

    static auto _hasTemplateDefaultValues(hif::BList<hif::Declaration> &templates) -> bool;
};

CppStandardRefineVisitor::CppStandardRefineVisitor(
    hif::System *system,
    hif::semantics::ILanguageSemantics *sem,
    hif::semantics::ILanguageSemantics *checkSem)
    : _factory(sem)
    , _sem(sem)
    , _checkSem(checkSem)
    , _fixedSet()
    , _recursionSet()
    , _system(system)
    , _methodsWithTPDefaultValue()
{
    // ntd
}

CppStandardRefineVisitor::~CppStandardRefineVisitor()
{
    for (auto s : _fixedSet) {
        s->removeProperty(hif::PROPERTY_CONSTEXPR);
    }

    for (auto s : _methodsWithTPDefaultValue) {
        for (const auto &template_declaration : s->templateParameters) {
            // Check if the template parameter is a type or a value.
            auto ttp = dynamic_cast<hif::TypeTP *>(template_declaration);
            auto vtp = dynamic_cast<hif::ValueTP *>(template_declaration);
            if (ttp) {
                delete ttp->setType(nullptr);
            } else if (vtp) {
                delete vtp->setValue(nullptr);
            }
        }
    }

    if (_needToReset) {
        hif::semantics::resetTypes(_system);
        hif::semantics::flushTypeCacheEntries();
        hif::manipulation::flushInstanceCache();
    }
}

auto CppStandardRefineVisitor::visitFunction(hif::Function &o) -> int
{
    GuideVisitor::visitFunction(o);

    if (o.checkProperty(hif::PROPERTY_CONSTEXPR)) {
        _fixedSet.insert(&o);
    }

    if (_hasTemplateDefaultValues(o.templateParameters)) {
        _methodsWithTPDefaultValue.insert(&o);
    }

    return 0;
}

auto CppStandardRefineVisitor::visitProcedure(hif::Procedure &o) -> int
{
    GuideVisitor::visitProcedure(o);

    if (_hasTemplateDefaultValues(o.templateParameters)) {
        _methodsWithTPDefaultValue.insert(&o);
    }

    return 0;
}

auto CppStandardRefineVisitor::visitLibraryDef(hif::LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }
    GuideVisitor::visitLibraryDef(o);
    return 0;
}

auto CppStandardRefineVisitor::visitView(hif::View &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }
    GuideVisitor::visitView(o);
    return 0;
}

auto CppStandardRefineVisitor::visitFunctionCall(hif::FunctionCall &o) -> int
{
    // Since before visit, ensure declaration of potential sub-declarations.
    // Ref. design: vhdl/openCores/des56 + hif2vp
    hif::semantics::updateDeclarations(&o, _sem);

    hif::FunctionCall::DeclarationType *originalDecl = hif::semantics::getDeclaration(&o, _sem);
    if (originalDecl == nullptr) {
        messageDebugAssert(originalDecl != nullptr, "Unable to find declaration", &o, _sem);
        return 0;
    }

    if (_recursionSet.find(originalDecl) != _recursionSet.end()) {
        messageWarning(std::string("Found unsupported recursive function: ") + o.getName(), nullptr, nullptr);
        return 0;
    }
    GuideVisitor::visitFunctionCall(o);

    if (_methodsWithTPDefaultValue.find(originalDecl) != _methodsWithTPDefaultValue.end() ||
        _hasTemplateDefaultValues(originalDecl->templateParameters)) {
        hif::manipulation::sortParameters(
            o.templateParameterAssigns, originalDecl->templateParameters, true, hif::manipulation::SortMissingKind::ALL,
            _sem);
    }

    if (!originalDecl->checkProperty(hif::PROPERTY_CONSTEXPR)) {
        return 0;
    }
    auto ld = dynamic_cast<hif::LibraryDef *>(originalDecl->getParent());
    if (ld != nullptr && ld->isStandard()) {
        raiseUniqueWarning("Found at least a call to a standard function inside a template "
                           " that cannot be simplified at the moment. The generated code will "
                           "not compile.");
        return 0;
    }

    hif::FunctionCall::DeclarationType *instDecl = hif::manipulation::instantiate(&o, _sem);

    // A fresh copy is required for safe manipulation.
    hif::FunctionCall::DeclarationType *decl = hif::copy(instDecl);
    const bool can_replace                   = originalDecl->getParent() != nullptr;
    if (can_replace) {
        originalDecl->replace(decl);
        hif::semantics::updateDeclarations(decl, _sem);
        decl->replace(originalDecl);
    }
    hif::semantics::mapDeclarationsInTree(decl, instDecl, decl, _sem);

    for (const auto &parameter_assign : o.parameterAssigns) {
        for (auto parameter : decl->parameters) {
            if (parameter_assign->getName() == parameter->getName()) {
                delete parameter->setValue(hif::copy(parameter_assign->getValue()));
                break;
            }
        }
    }

    hif::manipulation::SimplifyOptions opt;
    opt.simplify_constants           = true;
    opt.simplify_parameters          = true;
    opt.simplify_template_parameters = true;
    opt.simplify_statements          = true;
    opt.context                      = &o;
    hif::manipulation::simplify(decl, _sem, opt);

    hif::semantics::mapDeclarationsInTree(decl, decl, instDecl, _sem);

    auto state_table = decl->getStateTable();
    auto action      = state_table->states.front()->actions.back();

    hif::Value *v = nullptr;
    if (dynamic_cast<hif::Return *>(action) != nullptr) {
        auto ret = dynamic_cast<hif::Return *>(action);
        v        = ret->setValue(nullptr);
        o.replace(v);
        delete &o;
        delete decl;
    } else {
        messageError(
            "Found function which must be inlined with C++98, but "
            "currently cannot be inlined.",
            decl, _sem);
    }

    _recursionSet.insert(instDecl);
    v->acceptVisitor(*this);
    hif::manipulation::mapToNative(v, _sem, _checkSem);
    _recursionSet.erase(instDecl);
    return 0;
}

auto CppStandardRefineVisitor::visitProcedureCall(hif::ProcedureCall &o) -> int
{
    GuideVisitor::visitProcedureCall(o);

    auto originalDecl = hif::semantics::getDeclaration(&o, _sem);
    if (originalDecl == nullptr) {
        messageDebugAssert(originalDecl != nullptr, "Unable to find declaration", &o, _sem);
        return 0;
    }

    if (_methodsWithTPDefaultValue.find(originalDecl) != _methodsWithTPDefaultValue.end() ||
        _hasTemplateDefaultValues(originalDecl->templateParameters)) {
        hif::manipulation::sortParameters(
            o.templateParameterAssigns, originalDecl->templateParameters, true, hif::manipulation::SortMissingKind::ALL,
            _sem);
    }

    return 0;
}

auto CppStandardRefineVisitor::visitTypeDef(hif::TypeDef &o) -> int
{
    if (o.templateParameters.empty()) {
        return 0;
    }

    // Take the reference to this TypeDef.
    hif::semantics::ReferencesSet references;
    hif::semantics::getReferences(&o, references, _sem, _system);

    // Insert typedef into a templated View.
    auto view = new hif::View();
    view->setName("behav");
    view->setLanguageID(hif::cpp); // The class must not extend sc_module.
    view->templateParameters.merge(o.templateParameters);
    view->setContents(new hif::Contents());
    view->setEntity(new hif::Entity());

    // Create a correspondent DesignUnit and replace TypeDef with it.
    auto du = new hif::DesignUnit();
    du->setName(hif::NameTable::getInstance()->getFreshName("HIF_Typedef"));
    du->views.push_back(view);
    du->addProperty(PROPERTY_TYPDEF_DESIGN_UNIT);

    o.replace(du);
    view->getContents()->declarations.push_back(&o);

    // Add class constructor and destructor.
    hif::HifFactory factory;
    hif::BList<hif::Parameter> pp;
    hif::BList<hif::Declaration> tp;
    view->getContents()->declarations.push_front(factory.classDestructor(du));
    view->getContents()->declarations.push_front(factory.classConstructor(du, pp, tp));

    _updateReferences(view, du->getName(), references);
    // ref. design: vhdl/openCores/avs_aes
    _needToReset = true;

    return 0;
}

auto CppStandardRefineVisitor::_updateReferences(
    hif::View *newDecl,
    const std::string &duName,
    hif::semantics::ReferencesSet &references) -> int
{
    for (auto it(references.begin()); it != references.end(); ++it) {
        auto tRef = dynamic_cast<hif::TypeReference *>(*it);
        messageAssert(tRef != nullptr, "Unexpected case", *it, _sem);

        auto vr = new hif::ViewReference();
        vr->setDesignUnit(duName);
        vr->setName(newDecl->getName());
        vr->templateParameterAssigns.merge(tRef->templateParameterAssigns);
        hif::semantics::setDeclaration(vr, newDecl);

        if (tRef->getInstance() == nullptr) {
            tRef->setInstance(vr);
            continue;
        }

        auto inst = tRef->getInstance();
        tRef->setInstance(vr);
        vr->setInstance(inst);
    }
    return 0;
}

auto CppStandardRefineVisitor::_hasTemplateDefaultValues(hif::BList<hif::Declaration> &templates) -> bool
{
    for (const auto &declaration : templates) {
        // Cast the declaration to type template parameter.
        auto ttp = dynamic_cast<hif::TypeTP *>(declaration);
        if (ttp && ttp->getType()) {
            return true;
        }
        // Cast the declaration to value template parameter.
        auto vtp = dynamic_cast<hif::ValueTP *>(declaration);
        if (vtp && vtp->getValue()) {
            return true;
        }
    }
    return false;
}

} // anonymous namespace.

void cpp98StandardRefinements(
    hif::System *o,
    hif2scParseLine & /*cLine*/,
    hif::semantics::ILanguageSemantics *sem,
    hif::semantics::ILanguageSemantics *checkSem)
{
    hif::application_utils::initializeLogHeader("HIF2SC", "cpp98StandardRefinements");

    {
        CppStandardRefineVisitor cppVis(o, sem, checkSem);
        o->acceptVisitor(cppVis);
    }

    hif::application_utils::restoreLogHeader();
}
