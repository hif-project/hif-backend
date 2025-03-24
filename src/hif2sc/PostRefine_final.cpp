/// @file PostRefine_final.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <algorithm>
#include <cinttypes>
#include <utility>

#include "hif2sc/PostRefineMethods.hpp"
#include "hif2sc/PrintMethods.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

using namespace hif;

namespace
{

// ///////////////////////////////////////////////////////////////////
// WhenSplitter
// ///////////////////////////////////////////////////////////////////
class WhenSplitter : public GuideVisitor
{
public:
    WhenSplitter(uint64_t maxDepth, hif::semantics::ILanguageSemantics *sem);
    ~WhenSplitter() override;

    auto hasSplit() const -> bool;

    auto visitLibraryDef(LibraryDef &o) -> int override;
    auto visitView(View &o) -> int override;

    auto visitWhen(When &o) -> int override;

private:
    static auto _isInConstantScope(When *o) -> bool;

    WhenSplitter(const WhenSplitter &)                     = delete;
    auto operator=(const WhenSplitter &) -> WhenSplitter & = delete;

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _factory;
    uint64_t _depth{0};
    const uint64_t _maxDepth;
    StateTable *_parentSt{nullptr};
    bool _hasSplit{false};
};

WhenSplitter::WhenSplitter(const uint64_t maxDepth, hif::semantics::ILanguageSemantics *sem)
    : _sem(sem)
    , _factory(sem)
    , _maxDepth(maxDepth)

{
    // ntd
}

WhenSplitter::~WhenSplitter()
{
    // ntd
}

auto WhenSplitter::hasSplit() const -> bool { return _hasSplit; }

auto WhenSplitter::visitLibraryDef(LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }
    GuideVisitor::visitLibraryDef(o);
    return 0;
}

auto WhenSplitter::visitView(View &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }
    GuideVisitor::visitView(o);
    return 0;
}

auto WhenSplitter::visitWhen(When &o) -> int
{
    if (_isInConstantScope(&o)) {
        return 0;
    }
    if (_depth < _maxDepth) {
        StateTable *restore = _parentSt;
        if (_depth == 0) {
            // top when
            _parentSt = hif::getNearestParent<StateTable>(&o);
        }

        ++_depth;
        GuideVisitor::visitWhen(o);
        --_depth;

        _parentSt = restore;
        return 0;
    }

    // Sanity checks
    messageAssert(_parentSt != nullptr, "Unable to split global When.", &o, _sem);
    messageAssert(_parentSt->declarations.empty(), "Unable to split When in scope with local declarations.", &o, _sem);

    auto *parentSub = dynamic_cast<SubProgram *>(_parentSt->getParent());
    if (parentSub != nullptr && (!parentSub->parameters.empty() || !parentSub->templateParameters.empty())) {
        messageError("Unable to split When in subprogram with paramenters or tempaltes.", &o, _sem);
    }

    Scope *scope = hif::getNearestScope(_parentSt->getParent(), true, false, false);
    if (scope == nullptr || dynamic_cast<View *>(scope) != nullptr) {
        // no suitable scope
        return 0;
    }

    // Splitting:
    _hasSplit = true;

    Type *whenType = hif::semantics::getSemanticType(&o, _sem);
    messageAssert(whenType != nullptr, "Cannot type When", &o, _sem);

    auto fname = NameTable::getInstance()->getFreshName(_parentSt->getName(), "_when");

    // fcall
    FunctionCall *fCall =
        _factory.functionCall(fname, nullptr, _factory.noTemplateArguments(), _factory.noParameterArguments());
    o.replace(fCall);

    // func
    SubProgram *func = _factory.subprogram(hif::copy(whenType), fname, _factory.noTemplates(), _factory.noParameters());
    StateTable *st   = _factory.stateTable(fname, _factory.noDeclarations(), _factory.retStmt(&o));
    func->setStateTable(st);
    BList<Declaration> *decls = hif::objectGetDeclarationList(scope);
    decls->push_front(func);

    hif::semantics::setDeclaration(fCall, func);

    // Recall:
    WhenSplitter ws(_maxDepth, _sem);
    o.acceptVisitor(ws);

    return 0;
}

auto WhenSplitter::_isInConstantScope(When *o) -> bool
{
    auto *r = getNearestParent<Range>(o);
    if (r != nullptr) {
        return true;
    }
    auto *vtp = getNearestParent<ValueTP>(o);
    if (vtp != nullptr) {
        return true;
    }
    auto *ttp = getNearestParent<TypeTP>(o);
    if (ttp != nullptr) {
        return true;
    }
    auto *tpa = getNearestParent<TPAssign>(o);
    if (tpa != nullptr) {
        return true;
    }
    auto *cc = getNearestParent<Const>(o);
    return cc != nullptr && cc->isDefine();
}

// /////////////////////////////////////////////////////////////////////////////
// IncludeRefineVisitor
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
    FinalRefineVisitor(
        System *root,
        semantics::ILanguageSemantics *sem,
        bool useHdtlib,
        bool useCpp98,
        uint64_t maxWhen,
        std::string sourcesExtension,
        std::string headerExtension);
    ~FinalRefineVisitor() override;

    /// @name Scope-related visits.
    /// @{
    auto visitDesignUnit(DesignUnit &o) -> int override;
    auto visitLibraryDef(LibraryDef &o) -> int override;
    auto visitSystem(System &o) -> int override;
    auto visitView(View &o) -> int override;
    /// @}

    /// @name Symbol-related visits.
    /// (1) Collect include of declaration scope.
    /// (2) Prefix the symbol with declaration scope if necessary.
    /// @{
    auto visitInstance(Instance &o) -> int override;
    auto visitViewReference(ViewReference &o) -> int override;
    /// @}

    /// @name Special cases.
    /// @{
    auto visitCast(Cast &o) -> int override;
    auto visitFile(File &o) -> int override;
    auto visitIdentifier(Identifier &o) -> int override;
    auto visitInt(Int &o) -> int override;
    auto visitLibrary(Library &o) -> int override;
    auto visitPortAssign(PortAssign &o) -> int override;
    auto visitString(String &o) -> int override;
    auto visitTypeReference(TypeReference &o) -> int override;
    auto visitWhen(When &o) -> int override;
    /// @}

    /// @name Printing fixes.
    /// @{
    auto visitExpression(Expression &o) -> int override;
    auto visitFor(For &o) -> int override;
    auto visitAggregate(Aggregate &o) -> int override;
    auto visitAggregateAlt(AggregateAlt &o) -> int override;
    auto visitFunctionCall(FunctionCall &o) -> int override;
    auto visitProcedureCall(ProcedureCall &o) -> int override;
    /// @}

    /// @brief Generate all the needed includes.
    void generateIncludes();

protected:
    template <typename T> bool _fixOverloadedOperatos(T *call);

    static auto _isInsideStandarLibDef(DesignUnit *du) -> bool;

    // Map scope - path-to-system. Same typedefs of findScopeDependencies
    using ScopedParents  = std::list<Scope *>;
    using ScopeRelations = std::map<Scope *, ScopedParents>;

    // Scope - required scopes by inner symbols
    using RequiredScopes = std::set<Scope *>;
    using Includes       = std::map<Scope *, RequiredScopes>;

private:
    // Disabled.
    FinalRefineVisitor(const FinalRefineVisitor &)                     = delete;
    auto operator=(const FinalRefineVisitor &) -> FinalRefineVisitor & = delete;

    /// @brief Return the nearest parent scope, skipping the ones contained into
    /// standard libraries.
    auto _getNearestScope(Object *start) -> Scope *;

    /// @brief For each symbol, get the scope containing its declaration and add
    /// it to dependencies of container scope, if they are different.
    void _addRequiredScopes(Object &o);

    /// @brief Push the include string in destination scope's library list.
    void _generateInclude(Scope *destination, BList<Library> &destLibraries, Scope *scopeToInclude);

    /// @brief For objects which will be printed in separate files, generate the
    /// string to include the proper header wrt scopes.
    auto _getImplementationInclude(Scope *scope) -> StringValue *;

    /// @brief Set property implementation includes if needed.
    void _addImplementationInclude(Scope *scope);

    /// @brief Add system library inclusion when needed.
    void _checkSystemLibrariesInclusion(Scope *scope);

    /// @brief Checks whether SystemC data types are used inside the scope.
    static auto _usesSystemC(Scope *scope) -> bool;

    /// @brief Add include of LibraryDef main header inside its View siblings.
    void _includeParentLibraryDefinitions(Object *o);

    /// @brief Add include of "hif_globals.hpp" (declaration scope) to current scope.
    void _addHifGlobalsLibrary(Object *decl);

    void _addGlobalLibraryInclusion(TypeReference *tr);

    auto _fixCombineTernary(When *o) -> bool;

    semantics::ILanguageSemantics *_sem;
    HifFactory _factory;

    /// @brief The general scope relations.
    ScopeRelations _scopeRelations;

    /// @brief Keep trace of current scope.
    Scope *_currentScope{nullptr};

    /// @brief The map of include to be filled during visits.
    /// Their generation is managed by public method generateIncludes().
    Includes _includes;

    /// @brief Set whether use hdtlib types.
    const bool _useHdtlib;

    /// @brief Set whether C++98 standard is required.
    const bool _useCpp98;

    /// @brief Set max nested when deep.
    const uint64_t _maxWhen;

    const std::string _sourcesExtension;
    const std::string _headerExtension;

    /// @brief The trash.
    hif::Trash _trash;

    /// @brief Pointer to system Object.
    System *_system;

    /// @brief The fake library def.
    LibraryDef *_fakeSystemLib{nullptr};
};

FinalRefineVisitor::FinalRefineVisitor(
    System *root,
    semantics::ILanguageSemantics *sem,
    const bool useHdtlib,
    const bool useCpp98,
    const uint64_t maxWhen,
    std::string sourcesExtension,
    std::string headerExtension)
    : _sem(sem)
    , _factory(sem)
    , _scopeRelations()
    , _includes()
    , _useHdtlib(useHdtlib)
    , _useCpp98(useCpp98)
    , _maxWhen(maxWhen)
    , _sourcesExtension(std::move(sourcesExtension))
    , _headerExtension(std::move(headerExtension))
    , _trash()
    , _system(root)

{
    hif::manipulation::findScopeDependencies(root, _scopeRelations);
}

FinalRefineVisitor::~FinalRefineVisitor() { _trash.clear(nullptr); }

auto FinalRefineVisitor::visitDesignUnit(DesignUnit &o) -> int
{
    Scope *restore = _currentScope;
    _currentScope  = &o;
    GuideVisitor::visitDesignUnit(o);
    _currentScope = restore;

    _addImplementationInclude(&o);

    return 0;
}

auto FinalRefineVisitor::visitLibraryDef(LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    _checkSystemLibrariesInclusion(&o);

    Scope *restore = _currentScope;
    _currentScope  = &o;
    GuideVisitor::visitLibraryDef(o);
    _currentScope = restore;

    _addImplementationInclude(&o);

    return 0;
}

auto FinalRefineVisitor::visitSystem(System &o) -> int
{
    // adding fake library used as library to include design units
    _fakeSystemLib = new LibraryDef();
    _fakeSystemLib->setName(NameTable::getInstance()->getFreshName("hif_fake_libraryDef"));
    _fakeSystemLib->setStandard(true);
    o.libraryDefs.push_front(_fakeSystemLib);

    _currentScope = &o;

    if (!o.declarations.empty() || !o.actions.empty()) {
        _checkSystemLibrariesInclusion(&o);
    }

    GuideVisitor::visitSystem(o);

    _addImplementationInclude(&o);

    return 0;
}

auto FinalRefineVisitor::visitView(View &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    _checkSystemLibrariesInclusion(&o);

    GuideVisitor::visitView(o);

    _includeParentLibraryDefinitions(&o);

    // Moves eventual view declarations in contents declarations
    Contents *c = o.getContents();
    if (c != nullptr) {
        o.declarations.merge(c->declarations);
        c->declarations.merge(o.declarations);
    } else {
        o.declarations.clear();
    }

    return 0;
}

auto FinalRefineVisitor::visitInstance(Instance &o) -> int
{
    GuideVisitor::visitInstance(o);

    _addRequiredScopes(o);

    return 0;
}

auto FinalRefineVisitor::visitViewReference(ViewReference &o) -> int
{
    GuideVisitor::visitViewReference(o);

    _addRequiredScopes(o);

    //    // Skip if inside inheritance list (should be already correct)
    //    if (o.isInBList())
    //    {
    //        View * p = dynamic_cast< View* >( o.getParent() );
    //        if (p != nullptr && &p->inheritance == reinterpret_cast<BList<ViewReference>*>(o.getBList())) return 0;
    //    }

    return 0;
}

auto FinalRefineVisitor::visitCast(Cast &o) -> int
{
    GuideVisitor::visitCast(o);
    if (!hif::objectIsNUllPointer(&o, _sem)) {
        return 0;
    }
    const hif::LanguageID lang = objectGetLanguage(&o);
    if (lang != hif::c && lang != hif::cpp) {
        return 0;
    }
    const std::string libName = lang == hif::c ? "stddef.h" : "cstddef";

    Library *libInclude = _factory.library("cstddef", nullptr, libName, false, true);

    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(libInclude, _currentScope, addOpt);

    return 0;
}

auto FinalRefineVisitor::visitFile(File &o) -> int
{
    GuideVisitor::visitFile(o);

    const hif::LanguageID lang = objectGetLanguage(&o);
    std::string libName;
    if (lang == hif::c) {
        libName = "stdio.h";
    } else {
        libName = "cstdio";
    }

    Library *libInclude = _factory.library("cstdio", nullptr, libName, false, true);

    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(libInclude, _currentScope, addOpt);

    return 0;
}

auto FinalRefineVisitor::visitString(String &o) -> int
{
    GuideVisitor::visitString(o);

    Library *stringLib = _factory.library("string", nullptr, "string", false, true);
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(stringLib, _currentScope, addOpt);

    return 0;
}

auto FinalRefineVisitor::visitTypeReference(TypeReference &o) -> int
{
    GuideVisitor::visitTypeReference(o);

    _addGlobalLibraryInclusion(&o);

    return 0;
}

auto FinalRefineVisitor::visitWhen(When &o) -> int
{
    GuideVisitor::visitWhen(o);
    _fixCombineTernary(&o);
    return 0;
}

auto FinalRefineVisitor::visitExpression(Expression &o) -> int
{
    GuideVisitor::visitExpression(o);

    Type *exprType = hif::semantics::getBaseType(hif::semantics::getSemanticType(&o, _sem), false, _sem);
    messageAssert(exprType != nullptr, "Cannot type expression", &o, _sem);

    auto *e = dynamic_cast<Event *>(exprType);
    if (e == nullptr) {
        return 0;
    }
    messageAssert(o.getOperator() == op_deref, "Unexpected operation", &o, _sem);

    auto *parent = dynamic_cast<ValueStatement *>(o.getParent());
    messageAssert(parent != nullptr, "Unsupported event expression location", o.getParent(), _sem);

    ProcedureCall *p = _factory.procedureCall(
        "notify", o.setValue1(nullptr), _factory.noTemplateArguments(), _factory.noParameterArguments());
    parent->replace(p);
    _trash.insert(parent);
    p->acceptVisitor(*this);

    return 0;
}

auto FinalRefineVisitor::visitFor(For &o) -> int
{
    GuideVisitor::visitFor(o);

    if (!_useHdtlib) {
        return 0;
    }

    Scope *parentScope = hif::getNearestScope(&o, true, false, false);
    messageAssert(parentScope != nullptr, "Cannot find parent scope", &o, _sem);
    for (BList<DataDeclaration>::iterator i = o.initDeclarations.begin(); i != o.initDeclarations.end();) {
        if (*i == o.initDeclarations.back()) {
            ++i;
            continue;
        }

        DataDeclaration *obj = *i;
        ++i;
        hif::manipulation::moveDeclaration(obj, parentScope, &o, _sem, "");
    }

    BList<Action>::iterator it(&o);
    for (BList<Action>::iterator i = o.initValues.begin(); i != o.initValues.end();) {
        if (*i == o.initValues.back()) {
            ++i;
            continue;
        }

        Action *obj = *i;
        i           = i.remove();
        it.insert_before(obj);
    }

    for (BList<Action>::iterator i = o.stepActions.begin(); i != o.stepActions.end();) {
        if (*i == o.stepActions.back()) {
            ++i;
            continue;
        }

        Action *obj = *i;
        i           = i.remove();
        o.forActions.push_back(obj);
    }

    return 0;
}

auto FinalRefineVisitor::visitAggregate(Aggregate &o) -> int
{
    // In case of global constants and defines aggregates initial value must
    // be unrolled.
    // Otherwise aggregate is better to be rolled in order to print as for.
    auto *c                    = dynamic_cast<Const *>(o.getParent());
    View *parentView           = hif::getNearestParent<View>(&o);
    const bool isInGlobalScope = (parentView == nullptr);
    const bool mustBeExpanded  = (c != nullptr && (c->isDefine() || isInGlobalScope));

    if (mustBeExpanded) {
        messageAssert(!c->isDefine() || isInGlobalScope, "Unexpected define in view scope", c, _sem);
        hif::manipulation::transformAggregateUnrollingAlts(&o, 2048, _sem, true);
        messageAssert(o.getOthers() == nullptr, "Cannot unroll aggregate as initial value of global constant", c, _sem);
    } else {
        hif::manipulation::transformAggregateRollingAlts(&o, true, _sem);
    }

    GuideVisitor::visitAggregate(o);
    return 0;
}

auto FinalRefineVisitor::visitAggregateAlt(AggregateAlt &o) -> int
{
    GuideVisitor::visitAggregateAlt(o);

    for (BList<Value>::iterator i = o.indices.begin(); i != o.indices.end(); ++i) {
        Value *ind = *i;
        hif::manipulation::simplify(ind, _sem);
    }

    return 0;
}

auto FinalRefineVisitor::visitFunctionCall(FunctionCall &o) -> int
{
    GuideVisitor::visitFunctionCall(o);

    // Refining calls to standard overloaded operators as plain expressions.
    if (_fixOverloadedOperatos(&o)) {
        return 0;
    }

    return 0;
}

auto FinalRefineVisitor::visitProcedureCall(ProcedureCall &o) -> int
{
    GuideVisitor::visitProcedureCall(o);

    // Refining calls to standard overloaded operators as plain expressions.
    if (_fixOverloadedOperatos(&o)) {
        return 0;
    }

    return 0;
}

auto FinalRefineVisitor::visitIdentifier(Identifier &o) -> int
{
    GuideVisitor::visitIdentifier(o);

    Identifier::DeclarationType *decl = hif::semantics::getDeclaration(&o, _sem);
    messageAssert(decl != nullptr, "Declaration not found", &o, _sem);

    auto *e = dynamic_cast<EnumValue *>(decl);
    if (e != nullptr) {
        _addHifGlobalsLibrary(e->getParent()->getParent());
    } else {
        _addHifGlobalsLibrary(decl->getParent());
    }

    return 0;
}

auto FinalRefineVisitor::visitInt(Int &o) -> int
{
    GuideVisitor::visitInt(o);

    if (o.getTypeVariant() != Type::NATIVE_TYPE && o.getTypeVariant() != Type::SYSTEMC_BIT_BITREF) {
        return 0;
    }

    auto *parent = getNearestParent<Const>(&o);
    if (parent != nullptr && parent->isDefine()) {
        return 0;
    }

    const hif::LanguageID lang = objectGetLanguage(&o);
    const std::string libName  = ((lang == hif::c) || _useCpp98) ? "stdint.h" : "cstdint";

    Library *intLib = _factory.library("cstdint", nullptr, libName, false, true);

    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(intLib, _currentScope, addOpt);

    return 0;
}

auto FinalRefineVisitor::visitLibrary(Library &o) -> int
{
    GuideVisitor::visitLibrary(o);

    // Skipping inclusions
    if (o.isInBList()) {
        return 0;
    }

    // Set standard flag when library def language is "c"
    LibraryDef *ld = hif::semantics::getDeclaration(&o, _sem);
    messageAssert(ld != nullptr, "Declaration not found", &o, _sem);

    if (ld->getLanguageID() == hif::c) {
        o.setStandard(true);
        auto *to = hif::getNearestParent<TypedObject>(&o);
        if (to != nullptr) {
            hif::semantics::resetTypes(to);
        }
    }

    return 0;
}

auto FinalRefineVisitor::visitPortAssign(PortAssign &o) -> int
{
    GuideVisitor::visitPortAssign(o);

    if (dynamic_cast<Cast *>(o.getValue()) != nullptr) {
        hif::semantics::getSemanticType(&o, _sem);
#ifndef NDEBUG
        Cast *c      = dynamic_cast<Cast *>(o.getValue());
        Type *opType = hif::semantics::getSemanticType(c->getValue(), _sem);
#endif
        messageDebug("Cast op type:", opType, _sem);
        messageError("Unexpected cast in portassign", &o, _sem);
    }

    return 0;
}

void FinalRefineVisitor::generateIncludes()
{
    for (auto it(_includes.begin()); it != _includes.end(); ++it) {
        BList<Library> *libs = objectGetLibraryList(it->first);
        messageAssert(libs != nullptr, "Not found library list", it->first, _sem);

        for (auto jt(it->second.begin()); jt != it->second.end(); ++jt) {
            _generateInclude(it->first, *libs, *jt);
        }
    }
}

auto FinalRefineVisitor::_isInsideStandarLibDef(DesignUnit *du) -> bool
{
    if (du == nullptr) {
        return false;
    }
    auto *parentLib = getNearestParent<LibraryDef>(du);
    while (parentLib != nullptr) {
        if (parentLib->isStandard()) {
            return true;
        }
        parentLib = getNearestParent<LibraryDef>(parentLib);
    }

    return false;
}

void FinalRefineVisitor::_addHifGlobalsLibrary(Object *declarationScope)
{
    auto *s = dynamic_cast<System *>(declarationScope);
    if (s == nullptr) {
        return;
    }
    if (s == _currentScope) {
        return;
    }

    Library *globLib = _factory.library(NameTable::getInstance()->hifGlobals(), nullptr, "", false, false);
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(globLib, _currentScope, addOpt);
}

void FinalRefineVisitor::_addGlobalLibraryInclusion(TypeReference *tr)
{
    TypeReference::DeclarationType *decl = hif::semantics::getDeclaration(tr, _sem);
    messageAssert(decl != nullptr, "Declaration not found", tr, _sem);

    auto *e = dynamic_cast<TypeDef *>(decl);
    if (e == nullptr) {
        return;
    }
    _addHifGlobalsLibrary(e->getParent());
}

auto FinalRefineVisitor::_fixCombineTernary(When *o) -> bool
{
    if (!o->isLogicTernary()) {
        return false;
    }
    o->setLogicTernary(false);

    Type *whenType = hif::semantics::getSemanticType(o, _sem);
    messageAssert(whenType != nullptr, "Cannot type When", o, _sem);
    messageAssert(o->alts.size() == 1UL, "Unexpected native when", o, _sem);

    WhenAlt *whenThen = o->alts.front();
    auto *whenElse    = new WhenAlt();
    whenElse->setCondition(hif::copy(whenThen->getCondition()));
    whenElse->setValue(o->setDefault(nullptr));
    o->alts.push_back(whenElse);

    whenThen->setCondition(_factory.expression(
        whenThen->setCondition(nullptr), op_case_eq, _factory.bitval(bit_one, _factory.bit(true, true, true))));
    whenElse->setCondition(_factory.expression(
        whenElse->setCondition(nullptr), op_case_eq, _factory.bitval(bit_zero, _factory.bit(true, true, true))));

    Type *t1 = hif::semantics::getBaseType(hif::semantics::getSemanticType(whenThen->getValue(), _sem), false, _sem);
    Type *t2 = hif::semantics::getBaseType(hif::semantics::getSemanticType(whenElse->getValue(), _sem), false, _sem);
    Real *r1 = dynamic_cast<Real *>(t1);
    Real *r2 = dynamic_cast<Real *>(t2);

    if (r1 != nullptr || r2 != nullptr) {
        RealValue *rv = _factory.realval(0.0);
        o->setDefault(rv);
        return true;
    }

    Type *newt1 = hif::copy(t1);
    Type *newt2 = hif::copy(t2);
    hif::typeSetSigned(newt1, false, _sem);
    hif::typeSetSigned(newt2, false, _sem);

    Value *a   = hif::copy(whenThen->getValue());
    Value *b   = hif::copy(whenElse->getValue());
    Value *bvx = nullptr;
    Bit *bt    = dynamic_cast<Bit *>(whenType);
    auto *bvt  = dynamic_cast<Bitvector *>(whenType);
    if (bt != nullptr) {
        bvx = _factory.bitval(hif::bit_x, hif::copy(bt));
    } else if (bvt != nullptr) {
        std::string::size_type size = std::string::size_type(hif::semantics::spanGetBitwidth(bvt->getSpan(), _sem));
        Bitvector *tmp              = hif::copy(bvt);
        if (size != 0) {
            delete tmp->setSpan(new Range(int64_t(size) - 1, 0));
        }
        if (size == 0) {
            size = 1;
        }
        bvx = _factory.bitvectorval(std::string(size, 'X'), tmp);
    } else {
        messageError("Unexpected type of when", whenType, _sem);
    }

    // expr: (~(a ^ b) | 'X') ^ (~a)
    Value *expr = _factory.expression(
        _factory.expression(_factory.expression(op_bnot, _factory.expression(a, op_bxor, b)), op_bor, bvx), op_bxor,
        _factory.expression(op_bnot, hif::copy(a)));

    if (bvt != nullptr) {
        expr = _factory.cast(hif::copy(whenType), expr);
    }

    o->setDefault(expr);

    auto *constA = dynamic_cast<ConstValue *>(hif::getChildSkippingCasts(a));
    if (constA != nullptr) {
        auto *constB = dynamic_cast<ConstValue *>(hif::getChildSkippingCasts(b));
        if (constB != nullptr) {
            hif::manipulation::simplify(expr, _sem);
        }
    }

    return true;
}

void FinalRefineVisitor::_includeParentLibraryDefinitions(Object *o)
{
    View *v = dynamic_cast<View *>(o);
    messageAssert(v != nullptr, "Unsupported case", o, _sem);

    Object *p = o->getParent()->getParent();
    auto *ld  = dynamic_cast<LibraryDef *>(p);

    if (ld == nullptr) {
        return;
    }

    Library *intLib = _factory.library(ld->getName(), nullptr, "", false, false);

    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.deleteIfNotAdded             = true;
    hif::manipulation::addUniqueObject(intLib, v->libraries, addOpt);
}

auto FinalRefineVisitor::_getNearestScope(Object *start) -> Scope *
{
    if (dynamic_cast<System *>(start) != nullptr) {
        return nullptr;
    }

    Scope *ret = nullptr;

    // Nested DesignUnit is already managed by common header.
    ret = hif::getNearestParent<DesignUnit>(start);
    if (ret != nullptr) {
        while (ret != nullptr) {
            auto *du = getNearestParent<DesignUnit>(ret);
            if (du == nullptr) {
                break;
            }
            messageAssert(!du->views.empty() && du->views.size() == 1, "Unexpected view list size", du, _sem);
            if (du->views.front()->isStandard()) {
                return nullptr;
            }
            ret = du;
        }
        auto *l = hif::getNearestParent<LibraryDef>(ret);
        if (l != nullptr && l->isStandard() && _sem->isNativeLibrary(l->getName())) {
            return nullptr;
        }
        return ret;
    }

    // Outside DesignUnit
    ret = hif::getNearestParent<LibraryDef>(start);
    if (ret != nullptr) {
        if (dynamic_cast<LibraryDef *>(ret)->isStandard()) {
            return nullptr;
        }
        return ret;
    }

    ret = hif::getNearestParent<System>(start);
    return ret;
}

void FinalRefineVisitor::_addRequiredScopes(Object &o)
{
    Declaration *decl = hif::semantics::getDeclaration(&o, _sem);
    if (decl == nullptr) {
        return;
    }

    Scope *scope = _getNearestScope(static_cast<Object *>(decl));
    if (scope == nullptr) {
        return;
    }

    if (scope == _currentScope) {
        return;
    }

    // Actually defined in a different scope.

    _includes[_currentScope].insert(scope);
}

auto FinalRefineVisitor::_getImplementationInclude(Scope *scope) -> StringValue *
{
    // Calculate paths to reach root (System) scope
    std::string dots = hif::backends::calculateIncludePath(scope, _system, _headerExtension, _sem);
    if (dots.empty()) {
        return nullptr;
    }

    // Removes hifGlobals
    std::string::size_type pos = dots.find(NameTable::getInstance()->hifGlobals());
    dots                       = dots.substr(0, pos);
    // Calculate paths from root to scope
    std::string dirs           = hif::backends::calculateIncludePath(_system, scope, _headerExtension, _sem);

    std::string ret = dots + "../inc/" + dirs;

    return new StringValue(ret);
}

void FinalRefineVisitor::_addImplementationInclude(Scope *scope)
{
    StringValue *inc = _getImplementationInclude(scope);
    if (inc == nullptr) {
        return;
    }
    scope->addProperty(PROPERTY_IMPLEMENTATION_INCLUDE, inc);
}

void FinalRefineVisitor::_checkSystemLibrariesInclusion(Scope *scope)
{
    switch (objectGetLanguage(scope)) {
    case hif::rtl:
    case hif::ams: {
        if (_useHdtlib) {
            Library *hdtlib = _factory.library("hdtlib", nullptr, "", false, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            addOpt.position                     = 0;
            hif::manipulation::addUniqueObject(hdtlib, scope, addOpt);
        }

        Library *systemc   = _factory.library("sc_core", nullptr, "systemc", false, true);
        Library *systemcDt = _factory.library("sc_dt", nullptr, "systemc", true, true);

        hif::manipulation::AddUniqueObjectOptions addOpt;
        addOpt.equalsOptions.checkOnlyNames = true;
        addOpt.deleteIfNotAdded             = true;
        addOpt.position                     = 0;
        hif::manipulation::addUniqueObject(systemc, scope, addOpt);
        hif::manipulation::addUniqueObject(systemcDt, scope, addOpt);
        break;
    }
    case hif::tlm: {
        if (_useHdtlib) {
            Library *hdtlib = _factory.library("hdtlib", nullptr, "", false, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.deleteIfNotAdded = true;
            addOpt.position         = 0;
            hif::manipulation::addUniqueObject(hdtlib, scope, addOpt);
        }

        Library *tlm = _factory.library("tlm", nullptr, "tlm.h", false, true);
        hif::manipulation::AddUniqueObjectOptions addOpt;
        addOpt.equalsOptions.checkOnlyNames = true;
        addOpt.deleteIfNotAdded             = true;
        addOpt.position                     = 0;
        hif::manipulation::addUniqueObject(tlm, scope, addOpt);

        Library *systemc   = _factory.library("sc_core", nullptr, "systemc", false, true);
        Library *systemcDt = _factory.library("sc_dt", nullptr, "systemc", true, true);
        hif::manipulation::AddUniqueObjectOptions addOpt2;
        addOpt2.equalsOptions.checkOnlyNames = true;
        addOpt2.deleteIfNotAdded             = true;
        addOpt2.position                     = 0;
        hif::manipulation::addUniqueObject(systemc, scope, addOpt2);
        hif::manipulation::addUniqueObject(systemcDt, scope, addOpt);

        break;
    }
    case hif::cpp: {
        if (_useHdtlib) {
            Library *hdtlib = _factory.library("hdtlib", nullptr, "", false, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            addOpt.position                     = 0;
            hif::manipulation::addUniqueObject(hdtlib, scope, addOpt);
        } else if (_usesSystemC(scope)) {
            Library *systemc   = _factory.library("sc_core", nullptr, "systemc", false, true);
            Library *systemcDt = _factory.library("sc_dt", nullptr, "systemc", true, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            addOpt.position                     = 0;
            hif::manipulation::addUniqueObject(systemc, scope, addOpt);
            hif::manipulation::addUniqueObject(systemcDt, scope, addOpt);
        } else {
            Library *cmath = _factory.library("cmath", nullptr, "cmath", false, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            addOpt.position                     = 0;
            hif::manipulation::addUniqueObject(cmath, scope, addOpt);
        }
        break;
    }
    case hif::psl: {
        if (_useHdtlib) {
            Library *hdtlib = _factory.library("hdtlib", nullptr, "", false, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            addOpt.position                     = 0;
            hif::manipulation::addUniqueObject(hdtlib, scope, addOpt);
        }

        if (_usesSystemC(scope)) {
            Library *systemc   = _factory.library("sc_core", nullptr, "systemc", false, true);
            Library *systemcDt = _factory.library("sc_dt", nullptr, "systemc", true, true);
            hif::manipulation::AddUniqueObjectOptions addOpt;
            addOpt.equalsOptions.checkOnlyNames = true;
            addOpt.deleteIfNotAdded             = true;
            addOpt.position                     = 0;
            hif::manipulation::addUniqueObject(systemc, scope, addOpt);
            hif::manipulation::addUniqueObject(systemcDt, scope, addOpt);
        }
        break;
    }
    case hif::c:
        break;
    default:
        break;
    }
}

auto FinalRefineVisitor::_usesSystemC(Scope *scope) -> bool
{
    HifTypedQuery<Bitvector> query0;
    HifTypedQuery<Bit> query1;
    HifTypedQuery<BitvectorValue> query2;
    HifTypedQuery<BitValue> query3;
    HifTypedQuery<Signed> query4;
    HifTypedQuery<Unsigned> query5;
    HifTypedQuery<Time> query6;
    HifTypedQuery<Event> query7;

    HifTypedQuery<Int> queryA;
    queryA.matchTypeVariant = true;
    queryA.typeVariant      = Type::SYSTEMC_INT_SC_INT;
    HifTypedQuery<Int> queryB;
    queryB.matchTypeVariant = true;
    queryB.typeVariant      = Type::SYSTEMC_INT_SC_BIGINT;
    HifTypedQuery<Int> queryC;
    queryC.matchTypeVariant = true;
    queryC.typeVariant      = Type::SYSTEMC_BITVECTOR_PROXY;
    HifTypedQuery<Int> queryD;
    queryD.matchTypeVariant = true;
    queryD.typeVariant      = Type::SYSTEMC_BITVECTOR_BASE;
    HifTypedQuery<Int> queryE;
    queryE.matchTypeVariant = true;
    queryE.typeVariant      = Type::SYSTEMC_BIT_BITREF;

    query0.setNextQueryType(&query1);
    query1.setNextQueryType(&query2);
    query2.setNextQueryType(&query3);
    query3.setNextQueryType(&query4);
    query4.setNextQueryType(&query5);
    query5.setNextQueryType(&query6);
    query6.setNextQueryType(&query7);
    query7.setNextQueryType(&queryA);
    queryA.setNextQueryType(&queryB);
    queryB.setNextQueryType(&queryC);
    queryC.setNextQueryType(&queryD);
    queryD.setNextQueryType(&queryE);

    query0.onlyFirstMatch     = true;
    query0.skipStandardScopes = true;

    std::list<Object *> results;
    hif::search(results, scope, query0);
    return !results.empty();
}

void FinalRefineVisitor::_generateInclude(Scope *destination, BList<Library> &destLibraries, Scope *scopeToInclude)
{
    std::string include = hif::backends::calculateIncludePath(destination, scopeToInclude, _headerExtension, _sem);
    if (include.empty()) {
        return;
    }

    auto libName = objectGetName(scopeToInclude);

    if (dynamic_cast<DesignUnit *>(scopeToInclude) != nullptr) {
        auto *du = dynamic_cast<DesignUnit *>(scopeToInclude);
        if (du->checkProperty(PROPERTY_TYPDEF_DESIGN_UNIT)) {
            // for typedefs design unit do not adding of include.
            return;
        }

        // Workaround to avoid check of declarations fails.
        libName = _fakeSystemLib->getName();
    }

    Scope *tmpScope = scopeToInclude;
    auto *du        = dynamic_cast<DesignUnit *>(scopeToInclude);
    if (du != nullptr && !du->views.empty()) {
        tmpScope = du->views.front();
    }
    const bool isSystem = (hif::declarationIsPartOfStandard(tmpScope));

    if (_isInsideStandarLibDef(du)) {
        // avoid including of views belonging to standard library defs.
        return;
    }

    // Add correspondent library.
    Library *lib = _factory.library(libName, nullptr, include, false, isSystem);
    messageDebugAssert(!include.empty(), "Unexpected case", destination, _sem);

    destLibraries.push_back(lib);
}

template <typename T> bool FinalRefineVisitor::_fixOverloadedOperatos(T *call)
{
    if (call->getInstance() == nullptr) {
        return false;
    }
    SubProgram *decl = hif::semantics::getDeclaration(call, _sem);
    if (decl == nullptr) {
        return false;
    }
    const Operator op = operatorFromPlainString(call->getName(), "__systemc_");

    const BList<ParameterAssign>::size_t paramNumber = call->parameterAssigns.size();
    if (op == hif::op_none) {
        return false;
    }
    messageAssert(paramNumber < 2, "Unexpected call", call, _sem);
    messageAssert(paramNumber != 0 || hif::operatorIsUnary(op), "Unexpected unary operator call", call, _sem);
    messageAssert(paramNumber != 1 || hif::operatorIsBinary(op), "Unexpected binary operator call", call, _sem);

    auto *expr = new Expression();
    expr->setOperator(op);
    expr->setValue1(call->setInstance(nullptr));
    if (paramNumber == 1) {
        expr->setValue2(call->parameterAssigns.front()->setValue(nullptr));
    }

    if (dynamic_cast<FunctionCall *>(call) != nullptr) {
        call->replace(expr);
    } else {
        // pcall
        auto *vs = new ValueStatement();
        vs->setValue(expr);
        call->replace(vs);
    }

    delete call;
    return true;
}

} // namespace

void postRefinementsFinalStep(System *o, hif2scParseLine &cLine, semantics::ILanguageSemantics *sem)
{
    hif::application_utils::initializeLogHeader("HIF2SC", "postRefinementsFinalStep");

    if (cLine.useHDTLib()) {
        hif::backends::addHifLibrary("hdtlib", nullptr, o, sem, true);
    }

    hif::semantics::UpdateDeclarationOptions dopt;
    dopt.error = true;
    hif::semantics::updateDeclarations(o, sem, dopt);

    if (cLine.getMaxWhen() != 0) {
        WhenSplitter ws(cLine.getMaxWhen(), sem);
        o->acceptVisitor(ws);
    }

    FinalRefineVisitor irVis(
        o, sem, cLine.useHDTLib(), cLine.useCpp98(), cLine.getMaxWhen(), cLine.getSourcesExtension(),
        cLine.getHeadersExtension());
    o->acceptVisitor(irVis);

    irVis.generateIncludes();

    hif::application_utils::restoreLogHeader();
}
