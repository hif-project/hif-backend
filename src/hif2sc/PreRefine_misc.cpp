/// @file PreRefine_misc.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <algorithm>

#include <hif/hif.hpp>

#include "hif2sc/PreRefineMethods.hpp"

using namespace hif;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#endif

// Macro for activate misc steps debug
//#define HIFSC_MISC_DEBUG

namespace
{

using BreakList = std::list<Break *>;

// ///////////////////////////////////////////////////////////////////
// Break fix
// ///////////////////////////////////////////////////////////////////
void _fixBreaks(System *root, hif::semantics::ILanguageSemantics * /*sem*/)
{
    BreakList res;
    hif::HifTypedQuery<Break> q;
    hif::search(res, root, q);

    for (auto *o : res) {
        if (o->getName() == NameTable::getInstance()->none()) {
            continue;
        }

        auto *parentAct = hif::getNearestParent<Action>(o);
        while (parentAct != nullptr && dynamic_cast<If *>(parentAct) != nullptr) {
            // skip If statement..
            parentAct = hif::getNearestParent<Action>(parentAct);
        }

        std::string n;
        if (parentAct == nullptr) {
            // may be referred to process name
            auto *parentSt = hif::getNearestParent<StateTable>(o);
            if (parentSt == nullptr) {
                continue;
            }
            n = parentSt->getName();
        } else {
            n = hif::objectGetName(parentAct);
        }
        if (n == NameTable::getInstance()->none()) {
            continue;
        }
        if (n != o->getName()) {
            continue;
        }
        // remove label! will be traslated as "break"
        o->setName(NameTable::getInstance()->none());
    }
}

// ///////////////////////////////////////////////////////////////////
// Support functions
// ///////////////////////////////////////////////////////////////////

auto check_object_method(Object *o, const HifQueryBase * /*unused*/) -> bool
{
    if (dynamic_cast<Wait *>(o) != nullptr) {
        return true;
    }
    if (dynamic_cast<Assign *>(o) != nullptr) {
        auto *ass = dynamic_cast<Assign *>(o);
        return (ass->getDelay() != nullptr);
    }

    return false;
}

// ///////////////////////////////////////////////////////////////////
// fix array constant initial value
// ///////////////////////////////////////////////////////////////////
class ArrayConstantValueVisitor : public GuideVisitor
{
public:
    ArrayConstantValueVisitor(hif::semantics::ILanguageSemantics *sem);
    ~ArrayConstantValueVisitor() override;

    auto visitIdentifier(Identifier &o) -> int override;
    auto visitFieldReference(FieldReference &o) -> int override;

private:
    ArrayConstantValueVisitor(const ArrayConstantValueVisitor &)                     = delete;
    auto operator=(const ArrayConstantValueVisitor &) -> ArrayConstantValueVisitor & = delete;

    auto _fixSymbol(Value *v, DataDeclaration *decl) -> bool;

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _f;
};

ArrayConstantValueVisitor::ArrayConstantValueVisitor(semantics::ILanguageSemantics *sem)
    : GuideVisitor()
    , _sem(sem)
    , _f(sem)
{
    // ntd
}

ArrayConstantValueVisitor::~ArrayConstantValueVisitor()
{
    // ntd
}

auto ArrayConstantValueVisitor::visitIdentifier(Identifier &o) -> int
{
    GuideVisitor::visitIdentifier(o);

    Identifier::DeclarationType *decl = hif::semantics::getDeclaration(&o, _sem);
    messageAssert(decl != nullptr, "Declaration not found", &o, _sem);

    _fixSymbol(&o, decl);

    return 0;
}

auto ArrayConstantValueVisitor::visitFieldReference(FieldReference &o) -> int
{
    GuideVisitor::visitFieldReference(o);

    FieldReference::DeclarationType *decl = hif::semantics::getDeclaration(&o, _sem);
    messageAssert(decl != nullptr, "Declaration not found", &o, _sem);

    auto *ddecl = dynamic_cast<DataDeclaration *>(decl);
    if (ddecl == nullptr) {
        return 0;
    }

    _fixSymbol(&o, ddecl);

    return 0;
}

auto ArrayConstantValueVisitor::_fixSymbol(Value *v, DataDeclaration *decl) -> bool
{
    if (decl->getValue() == nullptr) {
        return false;
    }

    Type *t = hif::semantics::getSemanticType(v, _sem);
    messageAssert(t != nullptr, "Cannot type value", v, _sem);
    Type *bt = hif::semantics::getBaseType(t, false, _sem);
    messageAssert(bt != nullptr, "Cannot find base type", t, _sem);

    auto *rec = dynamic_cast<Record *>(bt);
    if (rec != nullptr && hif::objectGetLanguage(v) != hif::c) {
        // cannot perform simplification since aggregate of type record
        // will be translated as calls of HifAggregateArray class.
        // Generated code is allowed only in c++ 11 since type is not POD.
        // ref. design: vhdl/gaisler/can_oc.
        return false;
    }

    // expand value
    Cast *cast = _f.cast(hif::copy(t), hif::copy(decl->getValue()));
    v->replace(cast);
    delete v;
    Value *simplified = hif::manipulation::simplify(cast, _sem);
    simplified->acceptVisitor(*this);

    return true;
}

// ///////////////////////////////////////////////////////////////////
// PreRefine_misc
// ///////////////////////////////////////////////////////////////////
class PreRefine_misc : public hif::GuideVisitor
{
public:
    using ObjectSet        = std::set<Object *>;
    using SimplifySet      = std::set<Object *>;
    using Declarations     = std::map<DataDeclaration *, std::pair<std::string, std::string>>;
    using SupportVariables = std::map<DataDeclaration *, DataDeclaration *>;

    /// @brief Default constructor.
    PreRefine_misc(
        System *system,
        hif::semantics::ILanguageSemantics *sem,
        hif::semantics::ILanguageSemantics *checkSem);

    /// @brief Destructor.
    ~PreRefine_misc() override;

    auto visitAssign(Assign &o) -> int override;
    auto visitBitValue(BitValue &o) -> int override;
    auto visitBitvectorValue(BitvectorValue &o) -> int override;
    auto visitContinue(Continue &o) -> int override;
    auto visitExpression(Expression &o) -> int override;
    auto visitFunction(Function &o) -> int override;
    auto visitGlobalAction(GlobalAction &o) -> int override;
    auto visitProcedure(Procedure &o) -> int override;
    auto visitStateTable(StateTable &o) -> int override;
    auto visitSwitch(Switch &o) -> int override;
    auto visitTypeDef(TypeDef &o) -> int override;
    auto visitTypeReference(TypeReference &o) -> int override;
    auto visitWait(hif::Wait &o) -> int override;
    auto visitWith(With &o) -> int override;
    auto visitView(View &o) -> int override;
    auto visitMember(Member &o) -> int override;
    auto visitParameterAssign(ParameterAssign &o) -> int override;
    auto visitConst(Const &o) -> int override;
    auto visitLibraryDef(LibraryDef &o) -> int override;

    /// @brief Used to simplify collected objects.
    auto AfterVisit(Object &o) -> int override;

private:
    hif::semantics::ILanguageSemantics *_sem;
    hif::semantics::ILanguageSemantics *_checkSem;

    SimplifySet _simplifySet;
    Declarations _declarations;
    hif::HifFactory _factory;

    /// @brief Collects objects to be deleted, to assure no segfaults.
    hif::Trash _trash;
    Object *_where{nullptr};

    using ReferencesMap = hif::semantics::ReferencesMap;
    using ReferencesSet = hif::semantics::ReferencesSet;
    ReferencesMap _refMap;

    SupportVariables _supportVariables;

    PreRefine_misc(const PreRefine_misc &)                     = delete;
    auto operator=(const PreRefine_misc &) -> PreRefine_misc & = delete;

    /// @name Switch and With related fixes.
    //@{

    auto _isNotGoodForSwitch(Value *v) -> bool;

    auto _needTranslationOfSwitchToif(Switch &o) -> bool;

    template <typename T> void _manageCaseSemantics(T *o, CaseSemantics cs);

    template <typename T> void _fixCaseSemantics(T *o);

    //@}

    /// @name StateTable-related fixes
    /// @{

    /// @brief Properly allocate the process-variable declarations depending on the
    /// process semantics (for more details see process_flavour)
    /// If the process semantics refers to VHDL or Verilog the declarations of
    /// process variables has to be moved to the parent node (Contents)
    /// for Verilog, we expect an empty hif::BList.
    auto _manageHdlProcessDeclarations(StateTable &o) -> bool;

    /// @brief If process maps a Verilog initial block, move it as a Procedure
    /// named 'start_of_simulation' to maintain equivalence.
    /// @return The created Procedure if fixed, nullptr otherwise.
    auto _manageInitialProcessDeclarations(StateTable *o) -> Procedure *;

    /// @brief Returns true if at least a wrong statement in reached from given root.
    /// Wrong statements are Wait (After and event type expressions when @p searchAllWrong).
    auto _checkWrongStatement(Object *root, bool searchAllWrong) -> bool;

    /// @}

    /// @brief Rename conflicting parameters (if any).
    void _renameConflictingParameters(SubProgram &o);

    /// @}

    /// @brief Wait-related methods
    /// @{

    auto _checkSigOrPortId(Identifier *id) -> bool;

    enum SensitivityKind { SENSITIVITY_BOTH, SENSITIVITY_POS, SENSITIVITY_NEG };

    void _manageWaitSensitivity(BList<Value> &sens, SensitivityKind kind);

    /// @}

    /// @name Concat related fixes.
    /// @{

    /// @brief In case of concat of Arrays, put a cast to Bitvector on each of them.
    auto _fixBitArrayConcat(Expression *o) -> bool;

    auto _fixAssignFromRecordValue(Assign *o) -> bool;

    /// @}

    /// @name View related fixes.
    /// @{

    /// @brief Fixes output ports initialization.
    void _fixOutputsInitialization(View *o);
    /// @brief For C++, manages non-compiletime templates, by passing them as
    /// ctor and dtor parameters.
    void _fixNonCompiletimeTemplates(View *o);

    /// @}

    /// @brief Insert library dependencies.
    void _addLibrary(Object *o, const char *libName);

    /// @brief Gets or creates start_of_simulation() inside given Contents.
    auto _getOrCreateStartOfSimulation(Contents *c) -> Procedure *;

    /// @name Static constant related fixes.
    /// @{

    static auto _getStaticConstScope(Const *o) -> Scope *;
    auto _fixConstArray(Const *o) -> bool;
    auto _fixStaticInitialization(Const *o) -> bool;

    /// @}

    /// @brief Fix assign to int slice
    auto _fixAssignToIntSlice(Assign *ass) -> bool;

    /// @brief Fix to fuctions which returned type is an array introducing a
    /// Record object containing the original array
    void _fixReturnArrayType(Function &o);
};

PreRefine_misc::PreRefine_misc(
    System *system,
    hif::semantics::ILanguageSemantics *sem,
    hif::semantics::ILanguageSemantics *checkSem)
    : GuideVisitor()
    , _sem(sem)
    , _checkSem(checkSem)
    , _simplifySet()
    , _declarations()
    , _factory(sem)
    , _trash()
    , _refMap()
    , _supportVariables()
{
    hif::semantics::getAllReferences(_refMap, sem, system);
}

PreRefine_misc::~PreRefine_misc() { _trash.clear(); }

auto PreRefine_misc::AfterVisit(Object &o) -> int
{
    _trash.clear(_where);
    if (_simplifySet.find(&o) == _simplifySet.end()) {
        return 0;
    }

    hif::semantics::resetTypes(&o);
    hif::manipulation::simplify(&o, _sem);

    return 0;
}

auto PreRefine_misc::_isNotGoodForSwitch(Value *v) -> bool
{
    // cast not allowed
    if (dynamic_cast<Cast *>(v) != nullptr) {
        return true;
    }

    // ranges not allowed
    auto *r = dynamic_cast<Range *>(v);
    if (r != nullptr) {
        return true;
    }

    // for types ask to check semantics
    Type *t = hif::semantics::getSemanticType(v, _sem);
    if (!_checkSem->isTypeAllowedAsCase(t)) {
        return true;
    }

    if (dynamic_cast<ConstValue *>(v) != nullptr) {
        return false;
    }

    if (dynamic_cast<hif::features::ISymbol *>(v) != nullptr) {
        Declaration *decl = hif::semantics::getDeclaration(v, _sem);

        if (dynamic_cast<EnumValue *>(decl) != nullptr) {
            return false;
        }

        auto *c = dynamic_cast<Const *>(decl);
        if (c != nullptr && c->isDefine()) {
            return false;
        }
    }

    return true;
}

auto PreRefine_misc::_needTranslationOfSwitchToif(Switch &o) -> bool
{
    Type *t = hif::semantics::getSemanticType(o.getCondition(), _sem);
    if (!_checkSem->isTypeAllowedAsCase(t)) {
        return true;
    }

    for (BList<SwitchAlt>::iterator i = o.alts.begin(); i != o.alts.end(); ++i) {
        for (BList<Value>::iterator j = (*i)->conditions.begin(); j != (*i)->conditions.end(); ++j) {
            if (_isNotGoodForSwitch(*j)) {
                return true;
            }
        }
    }

    return false;
}

template <typename T> void PreRefine_misc::_fixCaseSemantics(T *o)
{
    if (o->getCaseSemantics() == hif::CASE_LITERAL) {
        return;
    }

    Type *condType = hif::semantics::getSemanticType(o->getCondition(), _sem);
    messageAssert(condType != nullptr, "Cannot type condition", o, _sem);
    Type *condBaseType = hif::semantics::getBaseType(condType, false, _sem);
    messageAssert(condBaseType != nullptr, "Cannot type condition (2)", condType, _sem);

    // checks type of condition.
    // It may be different from logic vector or logic bit in case of manipulation
    // (e.g. DDT). In this case CASE_X and CASE_Z are mapped in CASE_LITERAL.
    bool isLogic = hif::typeIsLogic(condBaseType, _sem);
    if (isLogic) {
        return;
    }

    o->setCaseSemantics(hif::CASE_LITERAL);
}

template <typename T> void PreRefine_misc::_manageCaseSemantics(T *o, const CaseSemantics cs)
{
    if (o == nullptr) {
        return;
    }
    typedef typename T::AltType AltType;
    if (cs == hif::CASE_LITERAL) {
        return;
    }

    for (typename BList<AltType>::iterator i = o->alts.begin(); i != o->alts.end(); ++i) {
        AltType *alt = *i;
        auto *e      = dynamic_cast<Expression *>(hif::getChildSkippingCasts(alt->getCondition()));
        messageAssert(
            e != nullptr && e->getOperator() == op_case_eq, "Expected equality expression as condition",
            alt->getCondition(), _sem);
        Type *v1Type = hif::semantics::getSemanticType(e->getValue1(), _sem);
        messageAssert(v1Type != nullptr, "Cannot type description", e->getValue1(), _sem);
        Type *v1Base  = hif::semantics::getBaseType(v1Type, false, _sem);
        Bit *bb       = dynamic_cast<Bit *>(v1Base);
        bool isSigned = typeIsSigned(v1Base, _sem);

        FunctionCall *fc = _factory.functionCall(
            "hif_systemc_hif_caseXZ", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
            _factory.noTemplateArguments(),
            (_factory.parameterArgument("param1", e->setValue1(nullptr)),
             _factory.parameterArgument("param2", e->setValue2(nullptr)),
             _factory.parameterArgument("param3", _factory.boolval(cs == CASE_X))));

        if (bb == nullptr) {
            fc->parameterAssigns.push_back(_factory.parameterArgument("param4", _factory.boolval(isSigned)));
        }

        e->replace(fc);
        delete e;
    }

    auto *sys    = hif::getNearestParent<System>(o);
    Scope *scope = hif::getNearestScope(o, true, true, false);
    hif::backends::addHifLibrary("hif_systemc_hif_systemc_extensions", scope, sys, _sem, false);
}

auto PreRefine_misc::visitAssign(Assign &o) -> int
{
    GuideVisitor::visitAssign(o);
    if (_fixAssignFromRecordValue(&o)) {
        return 0;
    }
    if (_fixAssignToIntSlice(&o)) {
        return 0;
    }

    return 0;
}

auto PreRefine_misc::visitBitValue(BitValue &o) -> int
{
    GuideVisitor::visitBitValue(o);
    // Needed since some manipulations adds new vars which can have
    // init vals containing 'U'.
    // Ref design: verilog/openCores/gost89_ecb
    hif::manipulation::fixUnsupportedBits(&o, _sem, _checkSem);
    return 0;
}

auto PreRefine_misc::visitBitvectorValue(BitvectorValue &o) -> int
{
    GuideVisitor::visitBitvectorValue(o);
    // Needed since some manipulations adds new vars which can have
    // init vals containing 'U'.
    // Ref design: verilog/openCores/gost89_ecb
    hif::manipulation::fixUnsupportedBits(&o, _sem, _checkSem);
    return 0;
}

auto PreRefine_misc::visitContinue(Continue &o) -> int
{
    GuideVisitor::visitContinue(o);

    if (o.getName() == NameTable::getInstance()->none()) {
        return 0;
    }

    auto *parent = hif::getNearestParent<Action>(&o);

    while (parent != nullptr && dynamic_cast<If *>(parent) != nullptr) {
        parent = hif::getNearestParent<Action>(parent);
    }

    if (parent == nullptr) {
        return 0;
    }

    std::string n = hif::objectGetName(parent);
    if (n == NameTable::getInstance()->none()) {
        return 0;
    }
    if (n != o.getName()) {
        return 0;
    }
    o.setName(NameTable::getInstance()->none());
    return 0;
}

auto PreRefine_misc::visitExpression(Expression &o) -> int
{
    GuideVisitor::visitExpression(o);

    if (o.getOperator() == op_xor) {
        o.setOperator(op_bxor);
        raiseUniqueWarning("Found at least one logic XOR operation. "
                           "They have been translated as bitwise XOR operations.");
    }

    if (_fixBitArrayConcat(&o)) {
        return 0;
    }

    return 0;
}

auto PreRefine_misc::visitFunction(Function &o) -> int
{
    GuideVisitor::visitFunction(o);
    _renameConflictingParameters(o);
    _fixReturnArrayType(o);
    return 0;
}

auto PreRefine_misc::visitGlobalAction(GlobalAction &o) -> int
{
    GuideVisitor::visitGlobalAction(o);
    std::set<StateTable *> list;
    hif::manipulation::transformGlobalActions(&o, list, _sem);
    for (auto *i : list) {
        i->acceptVisitor(*this);
    }

    return 0;
}

auto PreRefine_misc::visitProcedure(Procedure &o) -> int
{
    GuideVisitor::visitProcedure(o);
    _renameConflictingParameters(o);
    return 0;
}

auto PreRefine_misc::visitStateTable(StateTable &o) -> int
{
    // Fixes that must be done before guide visitor
    Procedure *p = _manageInitialProcessDeclarations(&o);
    if (p != nullptr) {
        Object *restore = _where;
        _where          = &o;
        p->acceptVisitor(*this);
        _where = restore;
        return 0;
    }
    _manageHdlProcessDeclarations(o);

    GuideVisitor::visitStateTable(o);
    return 0;
}

auto PreRefine_misc::visitSwitch(Switch &o) -> int
{
    GuideVisitor::visitSwitch(o);
    _fixCaseSemantics(&o);

    const CaseSemantics cs = o.getCaseSemantics();
    o.setCaseSemantics(hif::CASE_LITERAL);

    if (!_needTranslationOfSwitchToif(o) && cs == hif::CASE_LITERAL) {
        return 0;
    }
    hif::manipulation::TransformCaseOptions translOpt;
    translOpt.fixCondition             = true;
    translOpt.splitCases               = (cs != hif::CASE_LITERAL);
    translOpt.simplify                 = false;
    translOpt.fixSignalOrPortCondition = true;
    If *transl                         = hif::manipulation::transformSwitchToIf(&o, translOpt, _sem);
    messageAssert(transl != nullptr, "Cannot map Switch to If", &o, _sem);
    _manageCaseSemantics(transl, cs);

    if (translOpt.createdVariable != nullptr) {
        translOpt.createdVariable->acceptVisitor(*this);
    }
    if (translOpt.createdAssign != nullptr) {
        translOpt.createdAssign->acceptVisitor(*this);
    }
    transl->acceptVisitor(*this);
    return 0;
}

auto PreRefine_misc::visitTypeDef(TypeDef &o) -> int
{
    GuideVisitor::visitTypeDef(o);

    if (dynamic_cast<Enum *>(o.getType()) != nullptr || dynamic_cast<Record *>(o.getType()) != nullptr) {
        return 0;
    }

    o.setOpaque(false);

    return 0;
}

auto PreRefine_misc::visitTypeReference(TypeReference &o) -> int
{

    Port *p = hif::getNearestParent<Port>(&o);
    auto *s = hif::getNearestParent<Signal>(&o);
    if (s == nullptr && p == nullptr) {
        return 0;
    }

    Type *t = hif::semantics::getBaseType(&o, false, _sem, false);
    auto *a = dynamic_cast<Array *>(t);
    if (a == nullptr) {
        return GuideVisitor::visitTypeReference(o);
    };

    hif::manipulation::PrefixTreeOptions popt;
    Type *expanded = hif::semantics::getPrefixedType(a, _sem, popt, &o);
    o.replace(expanded);
    delete &o;

    raiseUniqueWarning("Found at least one signal or port of type \"array\". "
                       "They have been translated as arrays of signals.");

    a->acceptVisitor(*this);

    return 0;
}

void _castToNativeIntIndex(Value *v, hif::semantics::ILanguageSemantics *sem)
{
    Type *t = hif::semantics::getSemanticType(v, sem);
    messageDebugAssert(t != nullptr, "Not found type of Value", v, sem);

    bool needCast = false;
    Int *i        = dynamic_cast<Int *>(t);
    if (i != nullptr) {
        if (i->getTypeVariant() != Type::NATIVE_TYPE) {
            needCast = true;
        } else {
            unsigned long long int size = hif::semantics::spanGetBitwidth(i->getSpan(), sem);
            if (size != 8ULL && size != 16ULL && size != 32ULL && size != 64ULL) {
                needCast = true;
            }
        }
    } else {
        needCast = true;
    }

    if (!needCast) {
        return;
    }

    hif::HifFactory f(sem);
    Cast *c = new Cast();
    c->setType(f.integer(nullptr, typeIsSigned(t, sem)));
    v->replace(c);
    c->setValue(v);

    hif::semantics::resetTypes(c);
    hif::manipulation::simplify(c, sem);
}

auto PreRefine_misc::visitMember(Member &o) -> int
{
    GuideVisitor::visitMember(o);

    if (o.getIndex() != nullptr) {
        _castToNativeIntIndex(o.getIndex(), _sem);
    }

    return 0;
}

auto PreRefine_misc::visitParameterAssign(ParameterAssign &o) -> int
{
    GuideVisitor::visitParameterAssign(o);

    ParameterAssign::DeclarationType *param = hif::semantics::getDeclaration(&o, _sem);
    messageAssert(param != nullptr, "Declaration not found", &o, _sem);

    // Cannot pass as reference to methods signals and ports.
    if (param->getDirection() != dir_out && param->getDirection() != dir_inout) {
        return 0;
    }

    hif::TerminalPrefixOptions opt;
    opt.recurseIntoFieldRefs = true;
    opt.recurseIntoMembers   = true;
    opt.recurseIntoSlices    = true;
    Value *v                 = hif::getChildSkippingCasts(o.getValue());
    v                        = hif::getTerminalPrefix(v, opt);
    auto *id                 = dynamic_cast<Identifier *>(v);
    messageAssert(id != nullptr, "Unsupporte case", o.getValue(), _sem);

    DataDeclaration *decl = hif::semantics::getDeclaration(id, _sem);
    messageAssert(decl != nullptr, "Declaration not found", id, _sem);

    auto *sig  = dynamic_cast<Signal *>(decl);
    Port *port = dynamic_cast<Port *>(decl);

    if (sig == nullptr && port == nullptr) {
        return 0;
    }
    // Assuming out and inout ports/signals are write by method.

    // case to fix

    // 1- check supported case
    if (_checkWrongStatement(param->getParent(), true)) {
        messageError(
            "Unsupported parameter with out/inout direction, assigned by"
            " signals and ports, in methods with waits/afters.",
            param->getParent(), _sem);
    }

    // 2- fixing supported case adding temporary variable
    // E.g.: foo(sig)
    // become:
    // tmp = sig
    // foo(tmp)
    // sig = tmp;
    if (_supportVariables.find(decl) == _supportVariables.end()) {
        auto varName  = hif::NameTable::getInstance()->getFreshName(decl->getName(), "_tmp");
        Variable *tmp = _factory.variable(hif::copy(decl->getType()), varName, hif::copy(decl->getValue()));
        hif::manipulation::addDeclarationInContext(tmp, decl, false);
        _supportVariables[decl] = tmp;
    }

    auto varName       = _supportVariables[decl]->getName();
    auto *parentAction = hif::getNearestParent<Action>(&o);
    BList<Action>::iterator it(parentAction);
    messageAssert(parentAction != nullptr, "Cannot find parent action", &o, _sem);
    Assign *assBefore = _factory.assignment(new Identifier(varName), new Identifier(decl->getName()));
    it.insert_before(assBefore);

    Assign *assAfter = _factory.assignment(new Identifier(decl->getName()), new Identifier(varName));
    it.insert_after(assAfter);

    id->setName(varName);
    hif::semantics::setDeclaration(id, _supportVariables[decl]);

    hif::semantics::getAllReferences(_refMap, _sem, assBefore);
    hif::semantics::getAllReferences(_refMap, _sem, assAfter);
    _refMap[decl].erase(id);
    _refMap[_supportVariables[decl]].insert(id);

    return 0;
}

auto PreRefine_misc::visitConst(Const &o) -> int
{
    GuideVisitor::visitConst(o);

    if (_fixConstArray(&o)) {
        return 0;
    }
    if (_fixStaticInitialization(&o)) {
        return 0;
    }

    return 0;
}

auto PreRefine_misc::visitLibraryDef(LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }
    GuideVisitor::visitLibraryDef(o);
    return 0;
}

auto PreRefine_misc::visitWait(Wait &o) -> int
{
    GuideVisitor::visitWait(o);

    bool hasCondition      = (o.getCondition() != nullptr);
    bool hasRepetitions    = (o.getRepetitions() != nullptr);
    bool hasTime           = (o.getTime() != nullptr);
    bool hasSensitivity    = (!o.sensitivity.empty());
    bool hasSensitivityPos = (!o.sensitivityPos.empty());
    bool hasSensitivityNeg = (!o.sensitivityNeg.empty());

    if (hasSensitivity) {
        _manageWaitSensitivity(o.sensitivity, SENSITIVITY_BOTH);
    }
    if (hasSensitivityPos) {
        _manageWaitSensitivity(o.sensitivityPos, SENSITIVITY_POS);
    }
    if (hasSensitivityNeg) {
        _manageWaitSensitivity(o.sensitivityNeg, SENSITIVITY_NEG);
    }

    if (hasRepetitions && !hasSensitivity && !hasSensitivityPos && !hasSensitivityNeg && !hasCondition && !hasTime) {
        return 0;
    }
    if (!hasRepetitions && !hasCondition) {
        return 0;
    }

    if (hasRepetitions) {
        // hasRepetision + something else...
        // translating as for()
        auto index   = hif::NameTable::getInstance()->getFreshName("i");
        For *forloop = new For();
        forloop->initDeclarations.push_back(_factory.variable(_factory.integer(), index, _factory.intval(0)));
        forloop->setCondition(_factory.expression(new Identifier(index), op_lt, o.setRepetitions(nullptr)));
        forloop->stepActions.push_back(_factory.assignment(
            new Identifier(index), _factory.expression(new Identifier(index), op_plus, _factory.intval(1))));

        o.replace(forloop);
        forloop->forActions.push_back(&o);
        o.acceptVisitor(*this);
        return 0;
    }

    if (hasCondition) {
        // hasCondition (maybe also with + something else...)
        // translating as while(!condition) + wait(sensitivity)
        hif::HifTypedQuery<Identifier> query;
        std::list<Identifier *> resultList;
        hif::search(resultList, o.getCondition(), query);
        Value *cond      = o.setCondition(nullptr);
        auto *constValue = dynamic_cast<ConstValue *>(cond);

        messageAssert(!resultList.empty() || constValue != nullptr, "Cannot refine condition.", &o, _sem);

        if (constValue != nullptr) {
            If *w =
                _factory.ifStmt(_factory.noActions(), _factory.ifAlt(_factory.expression(op_not, cond), new Return()));
            o.replace(w);
            delete &o;
        } else {
            auto *w = new While();
            w->setCondition(_factory.expression(op_not, cond));

            for (auto &i : resultList) {
                if (!_checkSigOrPortId(i)) {
                    continue;
                }
                Value *id = hif::copy(i);
                hif::manipulation::AddUniqueObjectOptions addOpt;
                addOpt.deleteIfNotAdded = true;
                hif::manipulation::addUniqueObject(id, o.sensitivity, addOpt);
            }

            o.replace(w);
            w->actions.push_back(&o);
            o.acceptVisitor(*this);
        }

        return 0;
    }

    return 0;
}

auto PreRefine_misc::visitWith(With &o) -> int
{
    GuideVisitor::visitWith(o);
    _fixCaseSemantics(&o);

    const CaseSemantics cs = o.getCaseSemantics();
    o.setCaseSemantics(hif::CASE_LITERAL);

    hif::manipulation::TransformCaseOptions translOpt;
    translOpt.fixCondition             = true;
    translOpt.splitCases               = (cs != hif::CASE_LITERAL);
    translOpt.simplify                 = false;
    translOpt.fixSignalOrPortCondition = true;
    Value *transl                      = hif::manipulation::transformWithToWhen(&o, translOpt, _sem);
    messageAssert(transl != nullptr, "Cannot map With to When", &o, _sem);
    When *when = dynamic_cast<When *>(transl);
    _manageCaseSemantics(when, cs);

    if (translOpt.createdVariable != nullptr) {
        translOpt.createdVariable->acceptVisitor(*this);
    }
    if (translOpt.createdAssign != nullptr) {
        translOpt.createdAssign->acceptVisitor(*this);
    }
    transl->acceptVisitor(*this);
    return 0;
}

auto PreRefine_misc::visitView(View &o) -> int
{
    GuideVisitor::visitView(o);

    if (hif::declarationIsPartOfStandard(&o)) {
        return 0;
    }

    _fixOutputsInitialization(&o);
    _fixNonCompiletimeTemplates(&o);

    return 0;
}

void PreRefine_misc::_addLibrary(Object *o, const char *libName)
{
    auto *root = hif::getNearestParent<System>(o);
    messageAssert(root != nullptr, "Cannot reach System", o, _sem);

    Scope *scope = hif::getNearestScope(o, false, true, false);
    if (dynamic_cast<Contents *>(scope) != nullptr) {
        scope = dynamic_cast<View *>(scope->getParent());
    }
    messageAssert(scope != nullptr, "Cannot reach scope", o, _sem);

    std::string nn(libName);

    LibraryDef *ld = _sem->getStandardLibrary(nn);
    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.position                     = 0U;
    hif::manipulation::addUniqueObject(ld, root->libraryDefs, addOpt);

    Library *ref = _factory.library(nn, nullptr, "", false, true);
    hif::manipulation::AddUniqueObjectOptions addOpt2;
    addOpt2.deleteIfNotAdded = true;
    hif::manipulation::addUniqueObject(ref, scope, addOpt2);
}

auto PreRefine_misc::_getOrCreateStartOfSimulation(Contents *c) -> Procedure *
{
    Procedure *p = nullptr;
    for (BList<Declaration>::iterator i = c->declarations.begin(); i != c->declarations.end(); ++i) {
        Declaration *decl = (*i);
        if (decl->getName() != "start_of_simulation") {
            continue;
        }
        p = dynamic_cast<Procedure *>(decl);
        messageAssert(p != nullptr, "Unexpected start_of_simulation() declaration type.", *i, _sem);
        break;
    }

    if (p == nullptr) {
        p = dynamic_cast<Procedure *>(
            _factory.subprogram(nullptr, "start_of_simulation", _factory.noTemplates(), _factory.noParameters()));
        auto *body = new StateTable();
        body->setName("start_of_simulation");
        auto *state = new State();
        state->setName("start_of_simulation");
        body->states.push_back(state);
        p->setStateTable(body);
        c->declarations.push_back(p);
    }

    return p;
}

auto PreRefine_misc::_getStaticConstScope(Const *o) -> Scope *
{
    if (o->isDefine()) {
        return nullptr;
    }
    auto *scope    = hif::getNearestParent<Scope>(o);
    auto *ldScope  = dynamic_cast<LibraryDef *>(scope);
    auto *sysScope = dynamic_cast<System *>(scope);
    if (ldScope == nullptr && sysScope == nullptr && o->isInstance()) {
        return nullptr;
    }

    return scope;
}

auto PreRefine_misc::_fixConstArray(Const *o) -> bool
{
    if (!o->isInstance() || o->isDefine()) {
        return false;
    }

    Type *t = hif::semantics::getBaseType(o->getType(), false, _sem);
    if (dynamic_cast<Array *>(t) == nullptr) {
        return false;
    }
    View *pv = hif::getNearestParent<View>(o);
    if (pv != nullptr) {
        auto *v = new Variable();
        v->setName(o->getName());
        v->setType(o->setType(nullptr));
        v->setValue(o->setValue(nullptr));

        for (auto *ref : _refMap[o]) {
            hif::semantics::setDeclaration(ref, v);
        }
        _refMap[v].insert(_refMap[o].begin(), _refMap[o].end());
        _refMap[o].clear();

        o->replace(v);
        delete o;
        v->acceptVisitor(*this);
    } else {
        // global const of array type: must ensure init val is constant!
        // ref. design: vhdl/gaisler/can_oc
        ArrayConstantValueVisitor v(_sem);
        o->getValue()->acceptVisitor(v);
        // TODO: call visitor?
    }

    return true;
}

auto PreRefine_misc::_fixStaticInitialization(Const *o) -> bool
{
    Scope *scope = _getStaticConstScope(o);
    if (scope == nullptr) {
        return false;
    }

    hif::semantics::SymbolList list;
    hif::semantics::collectSymbols(list, o->getValue(), _sem, true);
    bool toFix = false;
    for (auto *symbol : list) {
        Declaration *decl = hif::semantics::getDeclaration(symbol, _sem);
        // May be nullptr in case of Instance with Library.
        if (decl == nullptr) {
            continue;
        }

        auto *constDecl = dynamic_cast<Const *>(decl);
        if (constDecl == nullptr) {
            continue;
        }

        Scope *declScope = _getStaticConstScope(constDecl);
        if (declScope == nullptr || declScope == scope) {
            continue;
        }

        toFix = true;
        break;
    }

    if (!toFix) {
        return false;
    }

    auto *constFunc = new Function();
    constFunc->setName(o->getName());
    constFunc->setType(o->setType(nullptr));
    StateTable *st =
        _factory.stateTable(o->getName(), _factory.noDeclarations(), _factory.retStmt(o->setValue(nullptr)));
    constFunc->setStateTable(st);
    o->replace(constFunc);
    _refMap[constFunc];

    for (auto i = _refMap[o].begin(); i != _refMap[o].end(); ++i) {
        Object *ref = *i;
        if (dynamic_cast<Identifier *>(ref) != nullptr) {
            auto *fc = new FunctionCall();
            fc->setName(constFunc->getName());
            hif::semantics::setDeclaration(fc, constFunc);
            ref->replace(fc);
            delete ref;
            _refMap[constFunc].insert(fc);
        } else if (dynamic_cast<FieldReference *>(ref) != nullptr) {
            auto *fr = dynamic_cast<FieldReference *>(ref);
            auto *fc = new FunctionCall();
            fc->setName(constFunc->getName());
            fc->setInstance(fr->setPrefix(nullptr));
            hif::semantics::setDeclaration(fc, constFunc);
            ref->replace(fc);
            delete ref;
            _refMap[constFunc].insert(fc);
        } else {
            messageError("Unexpected case", ref, _sem);
        }
    }

    _refMap.erase(o);
    delete o;
    return true;
}

auto PreRefine_misc::_fixAssignToIntSlice(Assign *ass) -> bool
{
    auto *slice = dynamic_cast<Slice *>(ass->getLeftHandSide());
    if (slice == nullptr) {
        return false;
    }
    // Translating:
    // Int a;
    // a[5:3] = b;
    // as:
    // a = (a & 110011) | (((Int)b) << 3) & 001100)
    Type *prefixType     = hif::semantics::getSemanticType(slice->getPrefix(), _sem);
    Type *prefixBaseType = hif::semantics::getBaseType(prefixType, false, _sem);
    Int *intPrefixType   = dynamic_cast<Int *>(prefixBaseType);
    if (intPrefixType == nullptr) {
        return false;
    }

    Int *intType = hif::copy(intPrefixType);
    intType->setSigned(false);
    Value *ones = _factory.expression(hif::op_bnot, _factory.intval(0, intType));

    Value *leftBound  = hif::manipulation::assureSyntacticType(slice->getSpan()->setLeftBound(nullptr), _sem);
    Value *rightBound = hif::manipulation::assureSyntacticType(slice->getSpan()->setRightBound(nullptr), _sem);
    Value *leftShift  = _factory.expression(
        ones, hif::op_sla, _factory.expression(leftBound, hif::op_plus, _factory.intval(1, hif::copy(intType))));
    Value *rightShift = _factory.expression(
        hif::copy(ones), hif::op_sra,
        _factory.expression(hif::semantics::typeGetSpanSize(intType, _sem), hif::op_minus, rightBound));
    // 111...0000...111
    Value *mask_101 = _factory.expression(leftShift, hif::op_bor, rightShift);
    // 000...1111...000
    Value *mask_010 = _factory.expression(hif::op_bnot, hif::copy(mask_101));

    Value *prefix = slice->setPrefix(nullptr);
    ass->setLeftHandSide(prefix);

    // (a & 111...0000...111)
    Value *leftExpr  = _factory.expression(hif::copy(prefix), hif::op_band, mask_101);
    // (((Int)b) << 3) & 000...1111...000)
    Value *rightExpr = _factory.expression(
        _factory.expression(
            _factory.cast(hif::copy(intType), ass->setRightHandSide(nullptr)), hif::op_sla, hif::copy(rightBound)),
        hif::op_band, mask_010);
    Value *expr = _factory.expression(leftExpr, hif::op_bor, rightExpr);
    ass->setRightHandSide(expr);
    delete slice;

    return true;
}

void PreRefine_misc::_fixReturnArrayType(Function &o)
{
    Type *funType = o.getType();
    auto *tr      = dynamic_cast<TypeReference *>(funType);
    if (tr != nullptr && hif::declarationIsPartOfStandard(tr, _sem)) {
        return;
    }
    auto *arr = dynamic_cast<Array *>(hif::semantics::getBaseType(funType, false, _sem));
    if (arr == nullptr) {
        return;
    }

    // Create TypeDef for Record
    const std::string n = std::string(o.getName()) + "_return_type";
    std::string tdName  = hif::NameTable::getInstance()->getFreshName(n);
    TypeDef *retTd      = _factory.recordTypeDef(
        tdName, _factory.field(hif::copy(funType), "value", _sem->getTypeDefaultValue(funType, &o)));

    // Template parameters check
    for (auto &it : _refMap) {
        Declaration *dec = it.first;
        if (!o.templateParameters.contains(dec)) {
            continue;
        }
        retTd->templateParameters.push_back(hif::copy(dec));
    }
    hif::semantics::resetDeclarations(retTd);

    // Insert TypeDef into proper BList and set function type
    messageAssert(o.isInBList(), "Unexpected function declaration", &o, _sem);
    BList<Object>::iterator funcIt(&o);
    funcIt.insert_before(retTd);
    TypeReference *typeRef = _factory.typeRef(tdName);
    o.setType(typeRef);

    // Template parameters of TypeReference
    for (BList<Declaration>::iterator it = retTd->templateParameters.begin(); it != retTd->templateParameters.end();
         ++it) {
        Declaration *dec = *it;
        if (dynamic_cast<ValueTP *>(dec) != nullptr) {
            auto *vtp  = dynamic_cast<ValueTP *>(dec);
            auto *vtpa = new ValueTPAssign();
            vtpa->setName(vtp->getName());
            vtpa->setValue(new Identifier(vtp->getName()));
            typeRef->templateParameterAssigns.push_back(vtpa);
        } else if (dynamic_cast<TypeTP *>(dec) != nullptr) {
            auto *ttp  = dynamic_cast<TypeTP *>(dec);
            auto *ttpa = new TypeTPAssign();
            ttpa->setName(ttp->getName());
            auto *tyr = new TypeReference();
            tyr->setName(ttp->getName());
            ttpa->setType(tyr);
            typeRef->templateParameterAssigns.push_back(ttpa);
        }
    }

    // For each function ref reset type and add FieldReference
    for (auto *it : _refMap[&o]) {
        auto *val = dynamic_cast<Value *>(it);
        if (val == nullptr) {
            continue;
        }
        hif::semantics::resetTypes(val, false);
        auto *fr = new FieldReference();
        fr->setName("value");
        val->replace(fr);
        fr->setPrefix(val);
    }
    // Fix each Return of the Function
    // var Ret
    // return expr -> Ret.value = expr; return Ret
    std::list<Return *> retList;
    hif::HifTypedQuery<Return> query;
    hif::search(retList, &o, query);

    Variable *retVar = _factory.variable(
        _factory.typeRef(tdName), hif::NameTable::getInstance()->getFreshName("ret"),
        _sem->getTypeDefaultValue(retTd->getType(), retTd));
    o.getStateTable()->declarations.push_back(retVar);

    for (auto *ret : retList) {
        Value *returnedValue = ret->getValue();
        messageAssert(ret->isInBList(), "Unexpected return statement", ret, _sem);
        BList<Action>::iterator jt(ret);

        auto *ass = new Assign();
        ass->setRightHandSide(returnedValue);
        ass->setLeftHandSide(_factory.fieldRef(new Identifier(retVar->getName()), "value"));
        jt.insert_before(ass);
        ret->setValue(new Identifier(retVar->getName()));
    }

    // Update refMap
    hif::semantics::getAllReferences(_refMap, _sem, retTd);
    hif::semantics::getAllReferences(_refMap, _sem, typeRef);
}

auto PreRefine_misc::_manageInitialProcessDeclarations(StateTable *o) -> Procedure *
{
    if (o->getFlavour() != pf_initial) {
        return nullptr;
    }

    // already fixed
    if (dynamic_cast<Procedure *>(o->getParent()) != nullptr) {
        return nullptr;
    }

    // Initial process refine
    auto *contents = dynamic_cast<Contents *>(o->getParent());
    messageAssert(contents != nullptr, "Parent is not a Contents", o->getParent(), _sem);

    Procedure *procedure = _getOrCreateStartOfSimulation(contents);
    assert(procedure->getStateTable() != o);
    procedure->getStateTable()->declarations.merge(o->declarations);
    procedure->getStateTable()->states.front()->actions.merge(o->states.front()->actions);

    _trash.insert(o);
    return procedure;
}

auto PreRefine_misc::_checkWrongStatement(Object *root, bool searchAllWrong) -> bool
{
    hif::HifTypedQuery<Wait> q1;
    q1.sem                          = _sem;
    q1.checkInsideCallsDeclarations = true;
    q1.onlyFirstMatch               = true;

    hif::HifTypedQuery<Assign> q2;
    //hif::HifTypedQuery<Expression> q3;

    if (searchAllWrong) {
        q1.check_object_method = &check_object_method;

        q1.setNextQueryType(&q2);
        //q2.setNextQueryType(&q3);
    }

    std::list<Object *> result;
    hif::search(result, root, q1);

    return !result.empty();
}

auto PreRefine_misc::_manageHdlProcessDeclarations(StateTable &o) -> bool
{
    if (o.getFlavour() != pf_hdl && o.getFlavour() != pf_analog) {
        return false;
    }

    if (dynamic_cast<BaseContents *>(o.getParent()) == nullptr) {
        return false;
    }

    auto *contents = hif::getNearestParent<Contents>(&o);

    messageAssert(contents != nullptr, "Nearest contents not found", &o, _sem);

    BList<Declaration>::iterator d_it = o.declarations.begin();
    while (d_it != o.declarations.end()) {
        Declaration *dobj = *d_it;
        // visit
        dobj->acceptVisitor(*this);
        ++d_it;
        hif::manipulation::moveDeclaration(dobj, contents, &o, _sem, "");
    }

    hif::HifFactory f;
    f.setSemantics(_sem);

    if (_checkWrongStatement(&o, false)) {
        // Check whether the process code contains any wait statement
        hif::HifTypedQuery<Return> returnsQuery;
        std::list<Object *> resultList2;
        hif::search(resultList2, &o, returnsQuery);
        if (!resultList2.empty()) {
            messageError("This kind of statements is not supported yet.", &o, _sem);
        }

        o.setFlavour(pf_thread);
        auto *w = new While();
        w->setCondition(f.boolval(true));

        messageAssert(o.states.size() == 1, "Unexpected process", &o, _sem);
        State *state = o.states.front();
        w->actions.merge(state->actions);
        //Wait * wait = new Wait();
        //w->actions.push_back(wait);
        state->actions.push_back(w);
    } else {
        o.setFlavour(pf_method);
    }

    return true;
}

auto PreRefine_misc::_checkSigOrPortId(Identifier *id) -> bool
{
    Identifier::DeclarationType *decl = hif::semantics::getDeclaration(id, _sem);
    messageAssert(decl != nullptr, "In wait statement, identifier declaration not found", id, _sem);

    auto *s = dynamic_cast<Signal *>(decl);
    Port *p = dynamic_cast<Port *>(decl);

    return ((s != nullptr) || (p != nullptr));
}

void PreRefine_misc::_manageWaitSensitivity(BList<Value> &sens, PreRefine_misc::SensitivityKind kind)
{
    bool reqLibrary = false;

    for (BList<Value>::iterator i = sens.begin(); i != sens.end();) {
        auto *id = dynamic_cast<Identifier *>(*i);
        if (id == nullptr) {
            ++i;
            continue;
        }

        Identifier::DeclarationType *decl = hif::semantics::getDeclaration(id, _sem);
        messageAssert(decl != nullptr, "Canot find declaration.", id, _sem);
        if (dynamic_cast<Signal *>(decl) == nullptr && dynamic_cast<Port *>(decl) == nullptr) {
            ++i;
            continue;
        }

        i = i.remove();

        const char *fName = nullptr;

        switch (kind) {
        case SENSITIVITY_BOTH:
            fName = "hif_systemc_value_changed_event";
            break;
        case SENSITIVITY_POS:
            fName = "hif_systemc_posedge_event";
            break;
        case SENSITIVITY_NEG:
            fName = "hif_systemc_negedge_event";
            break;
        default:
            messageError("Unexpected switch case", nullptr, nullptr);
        }
        FunctionCall *fcall =
            _factory.functionCall(fName, id, _factory.noTemplateArguments(), _factory.noParameterArguments());

        sens.push_back(fcall);
        reqLibrary = true;
    }

    if (reqLibrary) {
        _addLibrary(sens.getParent(), "hif_systemc_sc_core");
    }
}

void PreRefine_misc::_renameConflictingParameters(SubProgram &o)
{
    View *view = hif::getNearestParent<View>(&o);
    if (view == nullptr) {
        return;
    }

    Contents *cont = view->getContents();
    if (cont == nullptr) {
        messageDebugAssert(cont != nullptr, "Unexpected view without contents", view, _sem);
        return;
    }

    // Formal parameters.
    for (BList<Parameter>::iterator spIt(o.parameters.begin()); spIt != o.parameters.end(); ++spIt) {
        bool needRename = false;

        for (BList<Declaration>::iterator it(view->templateParameters.begin()); it != view->templateParameters.end();
             ++it) {
            if ((*spIt)->getName() != (*it)->getName()) {
                continue;
            }
            needRename = true;
            break;
        }

        if (!needRename) {
            for (BList<Declaration>::iterator it(cont->declarations.begin()); it != cont->declarations.end(); ++it) {
                if ((*spIt)->getName() != (*it)->getName()) {
                    continue;
                }
                needRename = true;
                break;
            }
        }

        if (!needRename) {
            continue;
        }

        hif::manipulation::renameInScope(*spIt, _sem);
    }

    // Template parameters.
    for (BList<Declaration>::iterator spIt(o.templateParameters.begin()); spIt != o.templateParameters.end(); ++spIt) {
        bool needRename = false;

        for (BList<Declaration>::iterator it(view->templateParameters.begin()); it != view->templateParameters.end();
             ++it) {
            if ((*spIt)->getName() != (*it)->getName()) {
                continue;
            }
            needRename = true;
            break;
        }

        if (!needRename) {
            for (BList<Declaration>::iterator it(cont->declarations.begin()); it != cont->declarations.end(); ++it) {
                if ((*spIt)->getName() != (*it)->getName()) {
                    continue;
                }
                needRename = true;
                break;
            }
        }

        if (!needRename) {
            continue;
        }

        hif::manipulation::renameInScope(*spIt, _sem);
    }
}

auto _putCastToBitvector(Array *arr, Value *op) -> bool
{
    if (arr == nullptr) {
        return false;
    }
    Bit *b = dynamic_cast<Bit *>(arr->getType());
    if (b == nullptr) {
        return false;
    }

    auto *bv = new Bitvector();
    bv->setConstexpr(b->isConstexpr());
    bv->setLogic(b->isLogic());
    bv->setResolved(b->isResolved());
    bv->setSigned(arr->isSigned());
    bv->setSpan(hif::copy(arr->getSpan()));
    if (bv->getSpan()->getDirection() == dir_upto) {
        Range *r = bv->getSpan();
        r->setDirection(dir_downto);
        Value *v = r->setLeftBound(r->setRightBound(nullptr));
        r->setRightBound(v);
    }

    Cast *c = new Cast();
    c->setType(bv);
    op->replace(c);
    c->setValue(op);

    return true;
}

auto PreRefine_misc::_fixBitArrayConcat(Expression *o) -> bool
{
    if (o->getOperator() != op_concat) {
        return false;
    }

    auto *arr1 = dynamic_cast<Array *>(hif::semantics::getSemanticType(o->getValue1(), _sem));
    auto *arr2 = dynamic_cast<Array *>(hif::semantics::getSemanticType(o->getValue2(), _sem));

    bool changed = false;
    changed |= _putCastToBitvector(arr1, o->getValue1());
    changed |= _putCastToBitvector(arr2, o->getValue2());

    if (!changed) {
        return false;
    }

    Value *v = hif::manipulation::simplify(o, _sem);
    v->acceptVisitor(*this);
    return true;
}

auto PreRefine_misc::_fixAssignFromRecordValue(Assign *o) -> bool
{
    // fix for cpp98
    auto *rv = dynamic_cast<RecordValue *>(o->getRightHandSide());
    if (rv == nullptr) {
        return false;
    }

    ObjectSet objToCheck;
    auto *lhs     = dynamic_cast<hif::features::ISymbol *>(o->getLeftHandSide());
    Variable *var = nullptr;
    if (lhs != nullptr) {
        Declaration *decl = hif::semantics::getDeclaration(lhs->toObject(), _sem);
        messageAssert(decl != nullptr, "Declaration not found", lhs->toObject(), _sem);
        var = dynamic_cast<Variable *>(decl);
    }

    if (var == nullptr) {
        Type *lhsType = hif::semantics::getSemanticType(o->getLeftHandSide(), _sem);
        messageAssert(lhsType != nullptr, "Cannot type lhs", o->getLeftHandSide(), _sem);

        auto *support = new Variable();
        support->setName(NameTable::getInstance()->getFreshName("record_assign_support"));
        support->setType(hif::copy(lhsType));
        Scope *scope = hif::getNearestScope(o, true, false, false);
        messageAssert(scope != nullptr, "Cannot find parent suitable scope", o, _sem);
        BList<Declaration> *decls = hif::objectGetDeclarationList(scope);
        messageAssert(decls != nullptr, "Cannot find parent suitable scope (2)", o, _sem);
        decls->push_back(support);
        support->setValue(_sem->getTypeDefaultValue(lhsType, support));
        objToCheck.insert(support);

        BList<Action>::iterator it(o);
        for (BList<RecordValueAlt>::iterator i = rv->alts.begin(); i != rv->alts.end(); ++i) {
            RecordValueAlt *rva = *i;

            auto *fr = new FieldReference();
            fr->setName(rva->getName());
            fr->setPrefix(new Identifier(support->getName()));

            auto *ass = new Assign();
            ass->setLeftHandSide(fr);
            ass->setRightHandSide(rva->setValue(nullptr));
            it.insert_before(ass);
            objToCheck.insert(ass);
        }

        delete o->setRightHandSide(new Identifier(support->getName()));
        objToCheck.insert(o);
    } else {
        BList<Action>::iterator it(o);
        for (BList<RecordValueAlt>::iterator i = rv->alts.begin(); i != rv->alts.end(); ++i) {
            RecordValueAlt *rva = *i;

            auto *fr = new FieldReference();
            fr->setName(rva->getName());
            fr->setPrefix(hif::copy(o->getLeftHandSide()));

            auto *ass = new Assign();
            ass->setLeftHandSide(fr);
            ass->setRightHandSide(rva->setValue(nullptr));
            it.insert_before(ass);
            objToCheck.insert(ass);
        }

        _trash.insert(o);
    }

    for (auto *toCheck : objToCheck) {
        toCheck->acceptVisitor(*this);
    }

    return true;
}

void PreRefine_misc::_fixOutputsInitialization(View *o)
{
    if (o->getLanguageID() == hif::cpp) {
        return;
    }

    if (o->getContents() == nullptr) {
        auto *c = new Contents();
        c->setName("contents");
        o->setContents(c);
    }

    // Seems that SystemC does not initializes correctly output ports:
    // fix this by using start_of_simulation()

    // 1- Get or create start_of_simulation()
    Procedure *p = _getOrCreateStartOfSimulation(o->getContents());

    // 2- Push front all initials which are not the defaults.
    State *s = p->getStateTable()->states.front();
    for (BList<Port>::iterator i = o->getEntity()->ports.rbegin(); i != o->getEntity()->ports.rend(); --i) {
        Port *port = *i;
        if (port->getValue() == nullptr) {
            continue;
        }
        Value *v           = _sem->getTypeDefaultValue(port->getType(), port);
        bool needFix = !hif::equals(port->getValue(), v);
        delete v;
        if (!needFix) {
            continue;
        }
        auto *a = new Assign();
        a->setLeftHandSide(new Identifier(port->getName()));
        a->setRightHandSide(hif::copy(port->getValue()));
        s->actions.push_front(a);
    }

    // 3- If procedure state is empty, delete the procedure
    if (s->actions.empty() && o->getLanguageID() != hif::cpp && o->getLanguageID() != hif::c) {
        // In C & C++ the explicit scheduler calls the
        // start_of_simulation, so we cannot remove it.
        _trash.insert(p);
    }
}

void PreRefine_misc::_fixNonCompiletimeTemplates(View *o)
{
    if (o->getContents() == nullptr || o->getLanguageID() != hif::cpp) {
        return;
    }

    // Collecting vtp which must be pushed as new ctors params
    typedef std::list<ValueTP *> Templates;
    Templates templates;
    for (BList<Declaration>::iterator i = o->templateParameters.begin(); i != o->templateParameters.end(); ++i) {
        auto *vtp = dynamic_cast<ValueTP *>(*i);
        if (vtp == nullptr || vtp->isCompileTimeConstant()) {
            continue;
        }
        templates.push_back(vtp);
    }
    if (templates.empty()) {
        return;
    }

    // Fixing ctors.
    for (BList<Declaration>::iterator i = o->getContents()->declarations.begin();
         i != o->getContents()->declarations.end(); ++i) {
        auto *ctor = dynamic_cast<Function *>(*i);
        if (ctor == nullptr || ctor->getName() != NameTable::getInstance()->hifConstructor()) {
            continue;
        }

        // It is a ctor.
        // Add params with default values:
        for (auto *vtp : templates) {
            auto *p = new Parameter;
            p->setName(vtp->getName() + std::string("_"));
            p->setType(hif::copy(vtp->getType()));
            p->setValue(hif::copy(vtp->getValue()));
            ctor->parameters.push_back(p);
        }

        // Now fixing explicitally assigned tempaltes in ctor call.
        ReferencesSet &refSet = _refMap[ctor];
        for (auto j = refSet.begin(); j != refSet.end(); ++j) {
            auto *fc = dynamic_cast<FunctionCall *>(*j);
            messageAssert(fc != nullptr, "Unexpected ctor fcall reference", *j, _sem);
            if (fc->getInstance() == nullptr) {
                continue;
            }
            auto *inst = dynamic_cast<Instance *>(fc->getInstance());
            messageAssert(inst != nullptr, "Unexpected instance.", fc, _sem);
            auto *vr = dynamic_cast<ViewReference *>(inst->getReferencedType());
            messageAssert(vr != nullptr, "unexpected referenced type.", inst, _sem);
            for (BList<TPAssign>::iterator k = vr->templateParameterAssigns.begin();
                 k != vr->templateParameterAssigns.end(); ++k) {
                auto *vtpa = dynamic_cast<ValueTPAssign *>(*k);
                if (vtpa == nullptr) {
                    continue;
                }
                ValueTPAssign::DeclarationType *vtp = hif::semantics::getDeclaration(vtpa, _sem);
                messageAssert(vtp != nullptr, "Declaration not found", vtpa, _sem);
                if (vtp->isCompileTimeConstant()) {
                    continue;
                }

                auto *pa = new ParameterAssign;
                pa->setName(vtp->getName() + std::string("_"));
                pa->setValue(hif::copy(vtpa->getValue()));
                fc->parameterAssigns.push_back(pa);
            }
        }
    }
}

void _mapRangesToNative(hif::System *o, hif::semantics::ILanguageSemantics *sem)
{
    // Moving during misc visitor may be lead to break typing since GuideVisitor
    // may be not called on simblings (e.g. visiting of cast value before type of cast).
    // Ref. desing: verilog/openCores/or1200
    hif::HifTypedQuery<Range> q;
    hif::HifTypedQuery<Range>::Results list;
    hif::search(list, o, q);

    hif::semantics::ILanguageSemantics *checkSem = hif::semantics::SystemCSemantics::getInstance();

    for (auto *r : list) {
        if (r->getLeftBound() != nullptr) {
            hif::manipulation::mapToNative(r->getLeftBound(), sem, checkSem);
        }

        if (r->getRightBound() != nullptr) {
            hif::manipulation::mapToNative(r->getRightBound(), sem, checkSem);
        }
    }

    hif::semantics::resetTypes(o);
    hif::semantics::flushTypeCacheEntries();
    hif::semantics::resetDeclarations(o);
    hif::manipulation::flushInstanceCache();
    hif::manipulation::simplify(o, sem);
}

} // namespace

void fixMiscIssues(
    hif::System *o,
    hif::semantics::ILanguageSemantics *sem,
    hif::semantics::ILanguageSemantics *checkSem)
{
    hif::application_utils::initializeLogHeader("HIF2SC", "fixMiscIssues");

    // split concat targets
    hif::manipulation::SplitAssignTargetOptions splitOpt;
    splitOpt.splitConcats                    = true;
    splitOpt.splitRecordValueConcats         = true;
    splitOpt.splitArrays                     = true;
    splitOpt.removeSignalPortArrayParameters = true;
    hif::manipulation::splitAssignTargets(o, sem, splitOpt);

#ifdef HIFSC_MISC_DEBUG
    hif::writeFile("HIF2SC_MISC_01_after_split_assigns", o);
    messageAssert(hif::semantics::checkHif(o, sem) == 0, "Check failed: after split assigns", nullptr, nullptr);
    messageInfo("HIF2SC_MISC_01_after_split_assigns passed");
#endif

    // fix breaks
    _fixBreaks(o, sem);

#ifdef HIFSC_MISC_DEBUG
    hif::writeFile("HIF2SC_MISC_02_after_fix_breaks", o);
    messageAssert(hif::semantics::checkHif(o, sem) == 0, "Check failed: after fix breaks", nullptr, nullptr);
    messageInfo("HIF2SC_MISC_02_after_fix breaks passed");
#endif

    hif::manipulation::fixUnsupportedBits(o, sem, checkSem);

#ifdef HIFSC_MISC_DEBUG
    hif::writeFile("HIF2SC_MISC_03_after_fix_unsupported_bits", o);
    messageAssert(hif::semantics::checkHif(o, sem) == 0, "Check failed: after fix unsupported bits", nullptr, nullptr);
    messageInfo("HIF2SC_MISC_03_after_fix_unsupported_bits passed");
#endif

    // other fixes
    PreRefine_misc misc(o, sem, checkSem);
    o->acceptVisitor(misc);

#ifdef HIFSC_MISC_DEBUG
    hif::writeFile("HIF2SC_MISC_04_after_other_fixes", o);
    messageAssert(hif::semantics::checkHif(o, sem) == 0, "Check failed: after other fixes", nullptr, nullptr);
    messageInfo("HIF2SC_MISC_03_after_other_fixes passed");
#endif

    _mapRangesToNative(o, sem);

    hif::application_utils::restoreLogHeader();
}
