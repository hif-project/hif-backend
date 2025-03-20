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

using namespace hif;

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
    typedef std::set<hif::SubProgram *> SubProgramSet;

    /// @brief Default constructor.
    CppStandardRefineVisitor(
        System *system,
        hif::semantics::ILanguageSemantics *sem,
        hif::semantics::ILanguageSemantics *checkSem);
    /// @brief Destructor.
    virtual ~CppStandardRefineVisitor();

    /// @brief Remove template parameters from TypeDef, putting it inside a new
    /// templated DesignUnit.
    virtual int visitTypeDef(TypeDef &o);

    /// @brief Inlines functions which are marked as constexpr.
    virtual int visitFunctionCall(FunctionCall &o);
    virtual int visitProcedureCall(ProcedureCall &o);

    /// @name Collects methods in set to remove constexpr property
    /// and collets in other set to remove default values template parameter.
    /// @{

    virtual int visitFunction(Function &o);
    virtual int visitProcedure(Procedure &o);

    /// @}

    virtual int visitLibraryDef(LibraryDef &o);
    virtual int visitView(View &o);

private:
    hif::HifFactory _factory;
    hif::semantics::ILanguageSemantics *_sem;
    hif::semantics::ILanguageSemantics *_checkSem;
    SubProgramSet _fixedSet;
    SubProgramSet _recursionSet;
    bool _needToReset;
    System *_system;
    SubProgramSet _methodsWithTPDefaultValue;

    CppStandardRefineVisitor(const CppStandardRefineVisitor &);
    CppStandardRefineVisitor &operator=(const CppStandardRefineVisitor &);

    /// @brief Update the references.
    int _updateReferences(View *newDecl, const std::string &duName, hif::semantics::ReferencesSet &references);

    bool _hasTemplateDefaultValues(BList<Declaration> &templates);
};

CppStandardRefineVisitor::CppStandardRefineVisitor(
    System *system,
    hif::semantics::ILanguageSemantics *sem,
    hif::semantics::ILanguageSemantics *checkSem)
    : _factory(sem)
    , _sem(sem)
    , _checkSem(checkSem)
    , _fixedSet()
    , _recursionSet()
    , _needToReset(false)
    , _system(system)
    , _methodsWithTPDefaultValue()
{
    // ntd
}

CppStandardRefineVisitor::~CppStandardRefineVisitor()
{
    for (SubProgramSet::iterator i = _fixedSet.begin(); i != _fixedSet.end(); ++i) {
        SubProgram *s = *i;
        s->removeProperty(PROPERTY_CONSTEXPR);
    }

    for (SubProgramSet::iterator i = _methodsWithTPDefaultValue.begin(); i != _methodsWithTPDefaultValue.end(); ++i) {
        SubProgram *s = *i;
        for (BList<Declaration>::iterator j = s->templateParameters.begin(); j != s->templateParameters.end(); ++j) {
            Declaration *tp = *j;
            if (dynamic_cast<TypeTP *>(tp) != nullptr) {
                TypeTP *ttp = static_cast<TypeTP *>(tp);
                delete ttp->setType(nullptr);
            } else // value TP
            {
                ValueTP *vtp = static_cast<ValueTP *>(tp);
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

int CppStandardRefineVisitor::visitFunction(Function &o)
{
    GuideVisitor::visitFunction(o);

    if (o.checkProperty(PROPERTY_CONSTEXPR)) {
        _fixedSet.insert(&o);
    }

    if (_hasTemplateDefaultValues(o.templateParameters)) {
        _methodsWithTPDefaultValue.insert(&o);
    }

    return 0;
}

int CppStandardRefineVisitor::visitProcedure(Procedure &o)
{
    GuideVisitor::visitProcedure(o);

    if (_hasTemplateDefaultValues(o.templateParameters)) {
        _methodsWithTPDefaultValue.insert(&o);
    }

    return 0;
}

int CppStandardRefineVisitor::visitLibraryDef(LibraryDef &o)
{
    if (o.isStandard())
        return 0;
    GuideVisitor::visitLibraryDef(o);
    return 0;
}

int CppStandardRefineVisitor::visitView(View &o)
{
    if (o.isStandard())
        return 0;
    GuideVisitor::visitView(o);
    return 0;
}

int CppStandardRefineVisitor::visitFunctionCall(hif::FunctionCall &o)
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

    if (!originalDecl->checkProperty(PROPERTY_CONSTEXPR))
        return 0;
    LibraryDef *ld = dynamic_cast<LibraryDef *>(originalDecl->getParent());
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

    for (BList<ParameterAssign>::iterator i = o.parameterAssigns.begin(); i != o.parameterAssigns.end(); ++i) {
        for (BList<Parameter>::iterator j = decl->parameters.begin(); j != decl->parameters.end(); ++j) {
            if ((*i)->getName() != (*j)->getName())
                continue;
            delete (*j)->setValue(hif::copy((*i)->getValue()));
            break;
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

    StateTable *st = decl->getStateTable();
    Action *a      = st->states.front()->actions.back();
    Value *v       = nullptr;
    if (dynamic_cast<Return *>(a) != nullptr) {
        Return *ret = static_cast<Return *>(a);
        v           = ret->setValue(nullptr);
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

int CppStandardRefineVisitor::visitProcedureCall(ProcedureCall &o)
{
    GuideVisitor::visitProcedureCall(o);

    ProcedureCall::DeclarationType *originalDecl = hif::semantics::getDeclaration(&o, _sem);
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

int CppStandardRefineVisitor::visitTypeDef(TypeDef &o)
{
    if (o.templateParameters.empty())
        return 0;

    // Take the reference to this TypeDef.
    hif::semantics::ReferencesSet references;
    hif::semantics::getReferences(&o, references, _sem, _system);

    // Insert typedef into a templated View.
    View *view = new View();
    view->setName("behav");
    view->setLanguageID(cpp); // The class must not extend sc_module.
    view->templateParameters.merge(o.templateParameters);
    view->setContents(new Contents());
    view->setEntity(new Entity());

    // Create a correspondent DesignUnit and replace TypeDef with it.
    DesignUnit *du = new DesignUnit();
    du->setName(NameTable::getInstance()->getFreshName("HIF_Typedef"));
    du->views.push_back(view);
    du->addProperty(PROPERTY_TYPDEF_DESIGN_UNIT);

    o.replace(du);
    view->getContents()->declarations.push_back(&o);

    // Add class constructor and destructor.
    hif::HifFactory factory;
    BList<Parameter> pp;
    BList<Declaration> tp;
    view->getContents()->declarations.push_front(factory.classDestructor(du));
    view->getContents()->declarations.push_front(factory.classConstructor(du, pp, tp));

    _updateReferences(view, du->getName(), references);
    // ref. design: vhdl/openCores/avs_aes
    _needToReset = true;

    return 0;
}

int CppStandardRefineVisitor::_updateReferences(
    View *newDecl,
    const std::string &duName,
    hif::semantics::ReferencesSet &references)
{
    for (hif::semantics::ReferencesSet::iterator it(references.begin()); it != references.end(); ++it) {
        TypeReference *tRef = dynamic_cast<TypeReference *>(*it);
        messageAssert(tRef != nullptr, "Unexpected case", *it, _sem);

        ViewReference *vr = new ViewReference();
        vr->setDesignUnit(duName);
        vr->setName(newDecl->getName());
        vr->templateParameterAssigns.merge(tRef->templateParameterAssigns);
        hif::semantics::setDeclaration(vr, newDecl);

        if (tRef->getInstance() == nullptr) {
            tRef->setInstance(vr);
            continue;
        }

        ReferencedType *inst = tRef->getInstance();
        tRef->setInstance(vr);
        vr->setInstance(inst);
    }
    return 0;
}

bool CppStandardRefineVisitor::_hasTemplateDefaultValues(BList<Declaration> &templates)
{
    for (BList<Declaration>::iterator i = templates.begin(); i != templates.end(); ++i) {
        if (dynamic_cast<TypeTP *>(*i) != nullptr) {
            TypeTP *ttp = static_cast<TypeTP *>(*i);
            if (ttp->getType() == nullptr)
                continue;

            // found
            return true;
        } else // value TP
        {
            ValueTP *vtp = static_cast<ValueTP *>(*i);
            if (vtp->getValue() == nullptr)
                continue;

            // found
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
