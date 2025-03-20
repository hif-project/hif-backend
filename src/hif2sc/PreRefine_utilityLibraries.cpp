/// @file PreRefine_utilityLibraries.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <cmath>
#include <sstream>

#include <hif/hif.hpp>

#include "hif2sc/PreRefineMethods.hpp"
#include "hif2sc/globals.hpp"

using namespace hif;
using std::stringstream;

namespace
{

bool _isInstanceOfStandardMethod(Value *o)
{
    FunctionCall *fc = dynamic_cast<FunctionCall *>(o->getParent());
    if (fc != nullptr && fc->getInstance() == o) {
        if (fc->getName() == "hif_vhdl_event")
            return true;
        if (fc->getName() == "hif_systemc_event")
            return true;
        if (fc->getName() == "hif_vhdl_last_value")
            return true;
        if (fc->getName() == "hif_systemc_read")
            return true;
        if (fc->getName() == "hif_systemc_value_changed_event")
            return true;
        if (fc->getName() == "hif_systemc_posedge_event")
            return true;
        if (fc->getName() == "hif_systemc_negedge_event")
            return true;
    }

    ProcedureCall *pc = dynamic_cast<ProcedureCall *>(o->getParent());
    if (pc != nullptr && pc->getInstance() == o) {
        // ...
    }

    return false;
}

// /////////////////////////////////////////////////////////////////////////////
// Refine introducing utility libraries
// /////////////////////////////////////////////////////////////////////////////

class PreRefine_utilityLibraries : public hif::GuideVisitor
{

public:
    typedef std::map<Declaration *, std::string> FunctionMap;

    /// @brief Default constructor and destructor.
    PreRefine_utilityLibraries(System *root, hif::semantics::ILanguageSemantics *sem, const hif2scParseLine &cLine);
    virtual ~PreRefine_utilityLibraries();

    // TODO array assign, etc --> hif2sc_assign()

    virtual int visitAggregate(hif::Aggregate &o);
    virtual int visitAssign(hif::Assign &o);
    virtual int visitExpression(hif::Expression &o);
    virtual int visitFunctionCall(FunctionCall &o);
    virtual int visitIdentifier(hif::Identifier &o);
    virtual int visitLibraryDef(hif::LibraryDef &o);
    virtual int visitMember(hif::Member &o);
    virtual int visitProcedureCall(hif::ProcedureCall &o);
    virtual int visitSlice(hif::Slice &o);
    virtual int visitSystem(hif::System &o);
    virtual int visitView(hif::View &o);

    /// @brief Tells whether support libraries have been introduced.
    bool hasIntroducedLibraries();

private:
    // Disabled.
    PreRefine_utilityLibraries(const PreRefine_utilityLibraries &);
    PreRefine_utilityLibraries &operator=(const PreRefine_utilityLibraries &);

    /// @name Fixes.
    //@{

    bool _fixRealEquality(Expression *o);

    /// @brief Manages VHDL mod operator.
    bool _fixModOperator(Expression *o);

    bool _fixReverseOperator(Expression *o);

    /// @brief Manages size operator on string that are the map for cass to string.size().
    bool _fixSizeOperator(Expression *o);

    /// @brief Manages pow operator.
    bool _fixPowOperator(Expression *o);

    /// @brief Manages VHDL abs operator.
    bool _fixAbsOperator(Expression *o);

    /// @brief Manages overload of operator (e.g., for SIGNED type in VHDL std_logic_arith).
    bool _fixOverloadedOperator(Expression *o);

    /// @brief Manages reduce operators introducing the corresponding function call.
    bool _fixReduceOperator(Expression *o);

    /// @brief Manages relational operator between arrays, by introducing
    /// hif_arrayEquals()/hif_arrayNotEquals() methods.
    bool _fixArrayEquals(Expression *o);

    /// @brief Fix op_eq and op_neq for logic values
    bool _fixLogicEquality(Expression *o);

    /// @brief Manages shift operations between logic vectors.
    bool _fixShiftOperator(Expression *o);

    /// @brief Support method for fix array concat.
    Value *_checkArrayOfSizeOneCase(Value *op, Array *opType, Array *otherOpType);

    /// @brief Manages concat between arrays.
    bool _fixArrayConcat(Expression *o);

    /// @brief Adds required SystemC read() calls.
    void _manageReadCall(Value *o);

    /// @brief Fix slice hdtlib type with adding a call to range() method.
    bool _fixSlice(Slice *o);

    /// @brief Fix slice on arrays.
    bool _fixArraySlice(Slice *o);

    /// @name Assign fixes
    /// @{

    /// @brief manage assignment between two arrays not packed.
    /// @return The created ProcedureCall, or nullptr if nothing is done.
    ///
    bool _manageAssignBetweenArrays(Assign &o);

    bool _manageAssignToStringSlice(Assign *o);

    void _setAssignBounds(Value *target, ProcedureCall *ret, const char recursion);

    /// @brief Support method to get the correct source for hif_assign method.
    ProcedureCall *_makeStandardAssignProcedure(Value *target, Value *sourceValue);

    /// @brief Support method to get the correct source for hif_assign method.
    Value *_getHifAssignSource(Value *source);

    /// @brief: manage assignment to a slice, assuming that source and destination
    /// types are compatible.
    bool _manageAssignToSlice(Assign &o);

    /// @brief: manage assignment to a slice, assuming that source and destination
    /// types are compatible.
    bool _manageHdtlibSliceAssign(Assign *o);
    bool _manageHdtlibMemberAssign(Assign *o);

    /// @}

    /// @name: manage the assigns of array to aggregates. This operation
    /// is allowed in HifSemantics (and other languages for example VHDL)
    /// but not in systemc so these two function change hif three by invoking
    /// two functions defined in hif2scSupport library.
    ///
    /// There are two managed cases:
    /// 1) Assign of aggregate to NOT packed array:
    /// Managed by function <tt>_manageAggregateToNotPackedArrayAssign</tt>
    /// In many language is possible to create an aggregate variable and assign
    /// it many times. This function replace those assigns with function
    /// call hif2sc_make_array.
    ///
    /// Transformation example:
    /// @code
    /// int arr[ 3 ] = {1,1,a};
    /// hif2sc_make_array<int>(1).add(1).add(a).fill(arr);
    /// @endcode
    ///
    /// limitations: actually work only with intvalue ranges.
    ///
    /// 2) Assign of aggregate to packed array:
    /// Managed by function <tt>_manageAggregateToPackedArrayAssign</tt>
    /// In this case aggregate represent a bit vector value. The problem is
    /// that aggregate may be described with a list of alts and an a "others"
    /// that is a default value used for all others bits not specified in alts.
    /// So this fix explicit this aggregate case by calling an function of
    /// hif2scSupport library.
    ///
    /// Example:
    /// Hif Aggregate:
    ///     ALT (position 0, bitval 'x')
    ///     ALT (position 3, bitval 'z')
    ///     OTHERS (bitval '1')
    /// The assignment "var = aggregate" will transfom as follows:
    /// @code
    /// a = hif2sc_make_aggregate<a.size()>( sc_bit('1'),       // others
    ///                                      0, sc_bit('x'),    // first alt
    ///                                      3, sc_bit('z'),    // second alt
    ///                                     )
    /// @endcode
    ///
    /// assumptions: works only with packed array. May be similar with not packed
    /// but at the moment is not handled.
    ///
    /// @{

    bool _manageAggregateArray(Aggregate *o);
    DataDeclaration *_getAggregateParent(Aggregate *o);

    /// Create call for HifAggregateArray, HifAggregateBitVector, HifAggregateLogicVector
    /// @param callName set the desired class
    /// @param t used to distinguish between case Array and Vector
    /// @param agg The aggregate
    FunctionCall *_createAggregateCall(const std::string &callName, Type *t, Aggregate *agg);

    /// @}

    /// @name Other fixes.
    //@{

    bool _fixAfter(Assign *o);

    //@}

    /// @name Support creation common methods.
    //@{

    /// Adds the "hif_" + s library.
    void _addHifLibrary(const std::string &s, const bool standard = false);

    //@}

    /// @name Renaming related methods.
    //@{

    template <typename T> bool _fixFunctionWithPedix(T *o);
    template <typename T> bool _fixResolvedMethods(T *o);
    bool _isResolvedConflicting(
        const std::string &lib,
        SubProgram *decl,
        SubProgram *&resolvedSub,
        SubProgram *&unresolvedSub);
    SubProgram *_getResolvedUnresolvedSubprogram(
        LibraryDef *ld,
        const std::string &subName,
        const BListHost::size_t pos,
        const bool resolved);
    bool _inSignedUnsignedRelatedLibrary(const std::string &lib, const std::string &func);
    bool _inHDTLibRelatedLibrary(const std::string &lib, const std::string &func);

    //@}

    /// Pointer to root, needed to add support standard libraries.
    System *_root;

    /// @brief The current scope (Contents, View, LibraryDef, System) where library
    /// references will be added.
    Object *_scope;

    /// @brief Tells whether at least a library reference has been added.
    bool _introducedLibraries;

    const hif2scParseLine &_cLine;

    hif::semantics::ILanguageSemantics *_sem;
    hif::HifFactory _factory;
    hif::Trash _trash;
};

PreRefine_utilityLibraries::PreRefine_utilityLibraries(
    System *root,
    hif::semantics::ILanguageSemantics *sem,
    const hif2scParseLine &cLine)
    : _root(root)
    , _scope(nullptr)
    , _introducedLibraries(false)
    , _cLine(cLine)
    , _sem(sem)
    , _factory(sem)
    , _trash()
{
    // ntd
}

PreRefine_utilityLibraries::~PreRefine_utilityLibraries() { _trash.clear(); }

bool PreRefine_utilityLibraries::hasIntroducedLibraries() { return _introducedLibraries; }

int PreRefine_utilityLibraries::visitAggregate(hif::Aggregate &o)
{
    GuideVisitor::visitAggregate(o);
    if (_manageAggregateArray(&o))
        return 0;
    return 0;
}

int PreRefine_utilityLibraries::visitAssign(hif::Assign &o)
{
    GuideVisitor::visitAssign(o);

    if (_fixAfter(&o))
        return 0; // must be first fix!
    if (_manageAssignToSlice(o))
        return 0;
    if (_manageAssignBetweenArrays(o))
        return 0;
    if (_manageAssignToStringSlice(&o))
        return 0;

    if (_manageHdtlibSliceAssign(&o))
        return 0;
    if (_manageHdtlibMemberAssign(&o))
        return 0;

    return 0;
}

int PreRefine_utilityLibraries::visitExpression(hif::Expression &o)
{
    GuideVisitor::visitExpression(o);
    if (_fixReverseOperator(&o))
        return 0;
    if (_fixSizeOperator(&o))
        return 0;
    if (_fixModOperator(&o))
        return 0;
    if (_fixPowOperator(&o))
        return 0;
    if (_fixAbsOperator(&o))
        return 0;
    if (_fixReduceOperator(&o))
        return 0;
    if (_fixArrayEquals(&o))
        return 0; // TODO: check
    if (_fixLogicEquality(&o))
        return 0;
    if (_fixOverloadedOperator(&o))
        return 0;
    if (_fixShiftOperator(&o))
        return 0;
    if (_fixArrayConcat(&o))
        return 0;
    if (_fixRealEquality(&o))
        return 0;
    return 0;
}

int PreRefine_utilityLibraries::visitFunctionCall(FunctionCall &o)
{
    GuideVisitor::visitFunctionCall(o);
    if (_fixFunctionWithPedix(&o))
        return 0;
    if (_fixResolvedMethods(&o))
        return 0;
    return 0;
}

int PreRefine_utilityLibraries::visitMember(Member &o)
{
    GuideVisitor::visitMember(o);
    _manageReadCall(&o);
    return 0;
}

int PreRefine_utilityLibraries::visitSlice(hif::Slice &o)
{
    GuideVisitor::visitSlice(o);
    if (_fixSlice(&o))
        return 0;
    if (_fixArraySlice(&o))
        return 0;
    return 0;
}

int PreRefine_utilityLibraries::visitIdentifier(Identifier &o)
{
    GuideVisitor::visitIdentifier(o);
    _manageReadCall(&o);
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

int PreRefine_utilityLibraries::visitProcedureCall(hif::ProcedureCall &o)
{
    GuideVisitor::visitProcedureCall(o);
    if (_fixFunctionWithPedix(&o))
        return 0;
    if (_fixResolvedMethods(&o))
        return 0;

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

int PreRefine_utilityLibraries::visitView(hif::View &o)
{
    Object *restore = _scope;
    _scope          = &o;
    GuideVisitor::visitView(o);
    _scope = restore;
    return 0;
}

bool PreRefine_utilityLibraries::_fixRealEquality(Expression *o)
{
    if (_cLine.noRealEquals())
        return false;
    if (!hif::operatorIsEquality(o->getOperator()))
        return false;
    auto r1 = dynamic_cast<Real *>(hif::semantics::getBaseType(o->getValue1(), false, _sem));
    auto r2 = dynamic_cast<Real *>(hif::semantics::getBaseType(o->getValue2(), false, _sem));
    if (r1 == nullptr || r2 == nullptr)
        return false;

    auto fCall = _factory.functionCall(
        "hif_systemc_hif_equals", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
        _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", o->getValue1()), _factory.parameterArgument("param2", o->getValue2())));

    Value *v = fCall;
    if (o->getOperator() == hif::op_case_neq || o->getOperator() == hif::op_neq)
        v = _factory.expression(hif::op_not, v);

    o->replace(v);
    _addHifLibrary("systemc_hif_systemc_extensions");
    delete o;
    return true;
}

bool PreRefine_utilityLibraries::_fixModOperator(Expression *o)
{
    // Fine also for hdtlib.

    if (o->getOperator() != op_mod)
        return false;

    Type *retT = hif::semantics::getSemanticType(o->getValue1(), _sem);

    Type *numT = hif::semantics::getBaseType(hif::semantics::getSemanticType(o->getValue1(), _sem), false, _sem);
    Type *divT = hif::semantics::getBaseType(hif::semantics::getSemanticType(o->getValue2(), _sem), false, _sem);

    Value *num = o->setValue1(nullptr);
    Value *div = o->setValue2(nullptr);

    // introduce casts
    Real *r1            = dynamic_cast<Real *>(numT);
    Real *r2            = dynamic_cast<Real *>(divT);
    const bool realCast = r1 != nullptr || r2 != nullptr;
    if (realCast)
        raiseUniqueWarning("Found at least one mod operation between real numbers. "
                           "Casting the operands to integers.");

    Type *tcast = _factory.integer(_factory.range(63, 0));

    FunctionCall *mod = _factory.functionCall(
        "hif_systemc_hif_mod", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
        _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", _factory.cast(tcast, num)),
         _factory.parameterArgument("param2", _factory.cast(hif::copy(tcast), div))));

    _addHifLibrary("systemc_hif_systemc_extensions");

    o->replace(mod);
    hif::backends::addEventualCast(mod, hif::semantics::getSemanticType(mod, _sem), retT);
    delete o;

    return true;
}

bool PreRefine_utilityLibraries::_fixReverseOperator(Expression *o)
{
    if (o->getOperator() != op_reverse)
        return false;

    Type *exprType    = hif::semantics::getSemanticType(o, _sem);
    Type *valType     = hif::semantics::getSemanticType(o->getValue1(), _sem);
    Type *valBaseType = hif::semantics::getBaseType(valType, false, _sem);
    messageAssert(valBaseType != nullptr, "Cannot type value1", o->getValue1(), _sem);

    String *str         = dynamic_cast<String *>(valBaseType);
    const bool isString = str != nullptr;
    const bool isVector = hif::semantics::isVectorType(valBaseType, _sem);

    if (!isString && !isVector)
        return false;

    FunctionCall *fCall = _factory.functionCall(
        "hif_systemc_hif_reverse", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
        _factory.noTemplateArguments(), _factory.parameterArgument("param1", o->setValue1(nullptr)));

    _addHifLibrary("systemc_hif_systemc_extensions");
    o->replace(fCall);

    hif::backends::makeParametersAssignable(fCall, _sem, true);
    hif::backends::addEventualCast(fCall, hif::semantics::getSemanticType(fCall, _sem), exprType);
    delete o;

    // Fix parameters etc
    fCall->acceptVisitor(*this);

    return true;
}

bool PreRefine_utilityLibraries::_fixSizeOperator(Expression *o)
{
    if (o->getOperator() != op_size)
        return false;

    Type *exprType    = hif::semantics::getSemanticType(o, _sem);
    Type *valType     = hif::semantics::getSemanticType(o->getValue1(), _sem);
    Type *valBaseType = hif::semantics::getBaseType(valType, false, _sem);
    messageAssert(valBaseType != nullptr, "Cannot type value1", o->getValue1(), _sem);

    String *str = dynamic_cast<String *>(valBaseType);
    if (str == nullptr)
        return false;

    FunctionCall *fCall = _factory.functionCall(
        "hif_systemc_size", o->setValue1(nullptr), _factory.noTemplateArguments(), _factory.noParameterArguments());

    _addHifLibrary("systemc_string");
    o->replace(fCall);

    hif::backends::makeParametersAssignable(fCall, _sem, true);
    hif::backends::addEventualCast(fCall, hif::semantics::getSemanticType(fCall, _sem), exprType);
    delete o;

    // Fix parameters etc
    fCall->acceptVisitor(*this);

    return true;
}

bool PreRefine_utilityLibraries::_fixPowOperator(Expression *o)
{
    // Fine also for hdtlib.

    if (o->getOperator() != op_pow)
        return false;

    Type *retT = hif::semantics::getSemanticType(o->getValue1(), _sem);
    messageAssert(retT != nullptr, "Cannot type expression.", o, _sem);

    if (dynamic_cast<RealValue *>(o->getValue1()) != nullptr &&
        fabs(static_cast<RealValue *>(o->getValue1())->getValue() - 2.0) < 0.0000001)
        return false;

    if (dynamic_cast<IntValue *>(o->getValue1()) != nullptr && static_cast<IntValue *>(o->getValue1())->getValue() == 2)
        return false;

    Value *num = o->setValue1(nullptr);
    Value *exp = o->setValue2(nullptr);

    FunctionCall *call = _factory.functionCall(
        "hif_systemc_pow", _factory.libraryInstance("hif_systemc_cmath", false, true), _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", num), _factory.parameterArgument("param2", exp)));

    _addHifLibrary("systemc_cmath");

    o->replace(call);
    hif::backends::makeParametersAssignable(call, _sem, true);
    hif::backends::addEventualCast(call, hif::semantics::getSemanticType(call, _sem), retT);
    delete o;

    return true;
}

bool PreRefine_utilityLibraries::_fixAbsOperator(Expression *o)
{
    // Fine also for hdtlib

    if (o->getOperator() != op_abs)
        return false;
    Type *exprType = hif::semantics::getSemanticType(o, _sem);

    Type *t  = hif::semantics::getSemanticType(o->getValue1(), _sem);
    Type *bt = hif::semantics::getBaseType(t, false, _sem);
    messageAssert(bt != nullptr, "Expected type", bt, _sem);

    FunctionCall *abs = nullptr;
    if (dynamic_cast<Signed *>(bt) != nullptr) {
        // TODO may add check on the library used? both numeric_std and std_logic_arith
        abs = _factory.functionCall(
            "hif_vhdl__op_abs", _factory.libraryInstance("hif_vhdl_ieee_std_logic_arith", false, true),
            _factory.noTemplateArguments(), _factory.parameterArgument("param1", o->setValue1(nullptr)));
        _addHifLibrary("vhdl_ieee_std_logic_arith");
    } else if (dynamic_cast<Bitvector *>(bt) != nullptr) {
        abs = _factory.functionCall(
            "hif_vhdl__op_abs", _factory.libraryInstance("hif_vhdl_ieee_std_logic_signed", false, true),
            _factory.noTemplateArguments(), _factory.parameterArgument("param1", o->setValue1(nullptr)));
        _addHifLibrary("vhdl_ieee_std_logic_signed");
    } else if (dynamic_cast<Int *>(bt) != nullptr) {
        abs = _factory.functionCall(
            "hif_systemc_abs", _factory.libraryInstance("hif_systemc_cstdlib", false, true),
            _factory.noTemplateArguments(), _factory.parameterArgument("param1", o->setValue1(nullptr)));
        _addHifLibrary("systemc_cstdlib");
    } else if (dynamic_cast<Real *>(bt) != nullptr) {
        abs = _factory.functionCall(
            "hif_systemc_abs", _factory.libraryInstance("hif_systemc_cmath", false, true),
            _factory.noTemplateArguments(), _factory.parameterArgument("param1", o->setValue1(nullptr)));
        _addHifLibrary("systemc_cmath");
    } else
        messageError("Unexpected case", o, _sem);

    o->replace(abs);
    hif::backends::makeParametersAssignable(abs, _sem, true);
    hif::backends::addEventualCast(abs, hif::semantics::getSemanticType(abs, _sem), exprType);
    delete o;

    return true;
}

bool PreRefine_utilityLibraries::_fixOverloadedOperator(Expression *o)
{
    // Fine also for hdtlib

    if (o->getValue2() == nullptr)
        return false;

    const bool isSupportedRelational = (hif::operatorIsRelational(o->getOperator()));
    const bool isSupportedArith =
        (o->getOperator() == op_plus || o->getOperator() == op_minus || o->getOperator() == op_mult);

    if (!isSupportedRelational && !isSupportedArith)
        return false;

    Type *t1 = hif::semantics::getBaseType(hif::semantics::getSemanticType(o->getValue1(), _sem), false, _sem);
    Type *t2 = hif::semantics::getBaseType(hif::semantics::getSemanticType(o->getValue2(), _sem), false, _sem);

    Type *in1 = hif::typeGetNestedType(t1, _sem);
    Type *in2 = hif::typeGetNestedType(t2, _sem);

    const bool t1IsArith = (dynamic_cast<Signed *>(in1) != nullptr || dynamic_cast<Unsigned *>(in1) != nullptr);
    const bool t2IsArith = (dynamic_cast<Signed *>(in2) != nullptr || dynamic_cast<Unsigned *>(in2) != nullptr);

    const bool t1IsBv = (dynamic_cast<Bitvector *>(in1) != nullptr);
    const bool t2IsBv = (dynamic_cast<Bitvector *>(in2) != nullptr);

    const bool t1IsBit = (dynamic_cast<Bit *>(in1) != nullptr) && (static_cast<Bit *>(in1)->isLogic());
    const bool t2IsBit = (dynamic_cast<Bit *>(in2) != nullptr) && (static_cast<Bit *>(in2)->isLogic());

    if ((!t1IsArith && !t1IsBv && !t1IsBit) || (!t2IsArith && !t2IsBv && !t2IsBit))
        return false;

    // On bv/lv case equality is truly case equality. Fine, no fix to do.
    if (t1IsBv && t2IsBv && (o->getOperator() == op_case_eq || o->getOperator() == op_case_neq))
        return false;

    const bool isComparisonOperator = o->getOperator() == op_lt || o->getOperator() == op_le ||
                                      o->getOperator() == op_ge || o->getOperator() == op_gt;

    // For logic bits, only relationals have problems
    if ((t1IsBit || t2IsBit) && !isComparisonOperator)
        return false;

    std::string fName("hif_vhdl_");
    if (isComparisonOperator)
        fName = "hif_";
    std::string libName;

    if (o->getOperator() == op_lt)
        fName += "_op_lt";
    else if (o->getOperator() == op_gt)
        fName += "_op_gt";
    else if (o->getOperator() == op_le)
        fName += "_op_le";
    else if (o->getOperator() == op_ge)
        fName += "_op_ge";
    else if (o->getOperator() == op_plus)
        fName += "_op_plus";
    else if (o->getOperator() == op_minus)
        fName += "_op_minus";
    else if (o->getOperator() == op_mult)
        fName += "_op_mult";
    else if (o->getOperator() == op_eq)
        fName += "_op_eq";
    else if (o->getOperator() == op_neq)
        fName += "_op_neq";
    else if (o->getOperator() == op_case_eq)
        fName += "_op_eq";
    else if (o->getOperator() == op_case_neq)
        fName += "_op_neq";
    else
        messageError("Unexpected case", o, _sem);

    if (isComparisonOperator) {
        // all relationals!
        fName = "hif_systemc_" + fName;
        if (!t1IsBit || !t2IsBit) {
            hif::semantics::ILanguageSemantics::ExpressionTypeInfo info =
                _sem->getExprType(in1, in2, o->getOperator(), o);

            if (hif::typeIsSigned(info.operationPrecision, _sem))
                fName = fName + "_signed";
            else
                fName = fName + "_unsigned";
        }
        libName = "systemc_hif_systemc_extensions";
    } else if (dynamic_cast<Signed *>(in1) != nullptr || dynamic_cast<Signed *>(in2) != nullptr) {
        fName += "_signed";
        if (o->getOperator() == op_case_eq || o->getOperator() == op_case_neq)
            libName = "vhdl_ieee_numeric_std";
        else
            libName = "vhdl_ieee_std_logic_arith";
    } else if (dynamic_cast<Unsigned *>(in1) != nullptr || dynamic_cast<Unsigned *>(in2) != nullptr) {
        fName += "_unsigned";
        if (o->getOperator() == op_case_eq || o->getOperator() == op_case_neq)
            libName = "vhdl_ieee_numeric_std";
        else
            libName = "vhdl_ieee_std_logic_arith";
    } else if (dynamic_cast<Bitvector *>(in1) != nullptr || dynamic_cast<Bitvector *>(in2) != nullptr) {
        Bitvector *bv1 = static_cast<Bitvector *>(in1);
        Bitvector *bv2 = static_cast<Bitvector *>(in2);

        hif::semantics::ILanguageSemantics::ExpressionTypeInfo info = _sem->getExprType(bv1, bv2, o->getOperator(), o);

        if (hif::typeIsSigned(info.operationPrecision, _sem))
            libName = "vhdl_ieee_std_logic_signed";
        else
            libName = "vhdl_ieee_std_logic_unsigned";
    } else {
        messageError("Unexpected case.", o, _sem);
    }

    Type *const t = hif::semantics::getSemanticType(o, _sem);

    FunctionCall *call = _factory.functionCall(
        fName, _factory.libraryInstance(("hif_" + libName), false, true), _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", o->setValue1(nullptr)),
         _factory.parameterArgument("param2", o->setValue2(nullptr))));

    _addHifLibrary(libName);

    o->replace(call);

    if (o->getOperator() != op_mult) {
        hif::semantics::ILanguageSemantics::ExpressionTypeInfo tt = _sem->getExprType(t1, t2, o->getOperator(), o);

        if (!hif::equals(t1, tt.operationPrecision)) {
            Value *vv = call->parameterAssigns.front()->setValue(nullptr);
            Cast *c   = new Cast();
            c->setType(hif::copy(tt.operationPrecision));
            c->setValue(vv);
            call->parameterAssigns.front()->setValue(c);
        }

        if (!hif::equals(t2, tt.operationPrecision)) {
            Value *vv = call->parameterAssigns.back()->setValue(nullptr);
            Cast *c   = new Cast();
            c->setType(hif::copy(tt.operationPrecision));
            c->setValue(vv);
            call->parameterAssigns.back()->setValue(c);
        }
    }

    Type *baseType = hif::semantics::getBaseType(t, false, _sem);
    hif::backends::makeParametersAssignable(call, _sem, true);
    const bool added = hif::backends::addEventualCast(call, hif::semantics::getSemanticType(call, _sem), t);
    if (added && dynamic_cast<Bool *>(baseType) != nullptr) {
        // In case of relationals, the custom call returns a bit,
        // whilst the original code returned a bool.
        // Therefore we must convert the bit to bool.
        // Ref design: vhdl/ips/mephisto_core
        Value *newValue = _sem->explicitBoolConversion(call);
        call->replace(newValue);
        delete call;
    }
    delete o;

    return true;
}

bool PreRefine_utilityLibraries::_fixReduceOperator(Expression *o)
{
    // Fine also for hdtlib.

    const hif::Operator op = o->getOperator();
    if (!hif::operatorIsReduce(op))
        return false;

    // TODO manage nand, nor, xnor (from parent visit)

    Type *exprType  = hif::semantics::getSemanticType(o, _sem);
    Type *valueType = hif::semantics::getSemanticType(o->getValue1(), _sem);
    messageAssert(exprType != nullptr, "Cannot type expression", o, _sem);
    messageAssert(valueType != nullptr, "Cannot type value", o->getValue1(), _sem);

    Int *local = dynamic_cast<Int *>(valueType);
    if (local != nullptr && local->getTypeVariant() == hif::Type::NATIVE_TYPE &&
        hif::semantics::typeGetSpanBitwidth(local, _sem) != 0) {
        // Integer case.
        if (op == op_orrd) {
            o->setValue2(_factory.intval(0, hif::copy(local)));
            o->setOperator(op_case_neq);
        } else if (op == op_andrd) {
            o->setValue2(_factory.expression(hif::op_bnot, _factory.intval(0, hif::copy(local))));
            o->setOperator(op_case_eq);
        } else if (op == op_xorrd) {
            // call support method
            FunctionCall *call = _factory.functionCall(
                "hif_systemc_hif_xorrd", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
                _factory.noTemplateArguments(), _factory.parameterArgument("param1", o->setValue1(nullptr)));
            Cast *c = new Cast();
            c->setType(hif::copy(exprType));
            c->setValue(call);
            o->replace(c);
            delete o;
            _addHifLibrary("systemc_hif_systemc_extensions");
            hif::backends::makeParametersAssignable(call, _sem, true);
            return true;
        }

        Cast *c = new Cast();
        c->setType(hif::copy(exprType));
        o->replace(c);
        c->setValue(o);
        return true;
    }

    // Other cases (bv, lv, etc)
    std::string fName("hif_systemc_");
    if (op == op_orrd)
        fName += "or_reduce";
    else if (op == op_andrd)
        fName += "and_reduce";
    else if (op == op_xorrd)
        fName += "xor_reduce";

    FunctionCall *call = _factory.functionCall(
        fName, _factory.libraryInstance("hif_systemc_sc_core", false, true), _factory.noTemplateArguments(),
        _factory.noParameterArguments());

    Cast *c = _factory.cast(hif::copy(exprType), call);

    o->replace(c);
    call->setInstance(o->setValue1(nullptr));
    delete o;

    if (_cLine.useHDTLib())
        _addHifLibrary("systemc_hdtlib");
    else
        _addHifLibrary("systemc_sc_core");

    return true;
}

bool PreRefine_utilityLibraries::_fixArrayEquals(Expression *o)
{
    // Fine also for hdtlib.

    if (o->getOperator() != op_case_eq && o->getOperator() != op_case_neq)
        return false;
    messageAssert(o->getValue2() != nullptr, "Expected 2nd operand", o, _sem);

    Type *exprType = hif::semantics::getSemanticType(o, _sem);
    Type *t1       = hif::semantics::getSemanticType(o->getValue1(), _sem);
    t1             = hif::semantics::getBaseType(t1, false, _sem);
    Type *t2       = hif::semantics::getSemanticType(o->getValue2(), _sem);
    t2             = hif::semantics::getBaseType(t2, false, _sem);

    messageAssert(t1 != nullptr && t2 != nullptr, "Expected types", o, _sem);
    const bool negCond       = (o->getOperator() == op_neq || o->getOperator() == op_case_neq);
    const hif::Operator opEq = negCond ? hif::operatorGetInverse(o->getOperator()) : o->getOperator();

    Array *arr1 = dynamic_cast<Array *>(t1);
    Array *arr2 = dynamic_cast<Array *>(t2);
    if (arr1 == nullptr || arr2 == nullptr)
        return false;

    const long long bw1 = static_cast<long long>(hif::semantics::spanGetBitwidth(arr1->getSpan(), _sem));
    const long long bw2 = static_cast<long long>(hif::semantics::spanGetBitwidth(arr2->getSpan(), _sem));

    if (bw1 == 0 || bw2 == 0) {
        FunctionCall *fc = _factory.functionCall(
            "hif_systemc_hif_arrayEquals", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
            _factory.noTemplateArguments(),
            (_factory.parameterArgument("param1", o->setValue1(nullptr)),
             _factory.parameterArgument("param2", o->setValue2(nullptr))));

        o->replace(fc);
        delete o;

        _addHifLibrary("systemc_hif_systemc_extensions");
        hif::backends::makeParametersAssignable(fc, _sem, true);

        if (negCond) {
            Expression *e = new Expression();
            fc->replace(e);
            e->setOperator(op_not);
            e->setValue1(fc);
            e->acceptVisitor(*this);
        }
    } else if (bw1 != bw2) {
        messageError("Equals between arrays with different span", o, _sem);
    } else {
        Value *ret = nullptr;
        for (long long i = 0; i < bw1; ++i) {
            Member *m1 = _factory.member(hif::copy(o->getValue1()), new IntValue(i));
            Member *m2 = _factory.member(hif::copy(o->getValue2()), new IntValue(i));
            Cast *eq   = _factory.cast(hif::copy(exprType), _factory.expression(m1, opEq, m2));
            if (ret == nullptr) {
                ret = eq;
            } else {
                ret = _factory.expression(ret, hif::op_and, eq);
            }
        }
        o->replace(ret);
        if (negCond) {
            Expression *e = new Expression();
            ret->replace(e);
            e->setOperator(op_not);
            e->setValue1(ret);
            e->acceptVisitor(*this);
        } else {
            ret->acceptVisitor(*this);
        }
    }

    return true;
}

bool PreRefine_utilityLibraries::_fixLogicEquality(Expression *o)
{
    // Fine also for hdtlib

    if (o->getOperator() != op_eq && o->getOperator() != op_neq) // for verilog/bitvector/hif
        return false;
    Type *exprType = hif::semantics::getSemanticType(o, _sem);
    messageAssert(exprType != nullptr, "Cannot type expression.", o, _sem);

    Type *top1 = hif::semantics::getBaseType(hif::semantics::getSemanticType(o->getValue1(), _sem), false, _sem);
    messageAssert(top1 != nullptr, "Cannot type op1", o->getValue1(), _sem);

    Type *top2 = hif::semantics::getBaseType(hif::semantics::getSemanticType(o->getValue2(), _sem), false, _sem);
    messageAssert(top2 != nullptr, "Cannot type op2", o->getValue2(), _sem);

    Type *in1 = hif::typeGetNestedType(top1, _sem);
    Type *in2 = hif::typeGetNestedType(top2, _sem);

    const bool t1IsLogic = hif::typeIsLogic(in1, _sem);
    const bool t2IsLogic = hif::typeIsLogic(in2, _sem);

    if (!t1IsLogic && !t2IsLogic)
        return false;

    const bool isLogicArith =
        (dynamic_cast<Signed *>(in1) != nullptr || dynamic_cast<Unsigned *>(in1) != nullptr ||
         dynamic_cast<Signed *>(in2) != nullptr || dynamic_cast<Unsigned *>(in2) != nullptr);

    if (isLogicArith)
        return false; // falling into vhdl: logic arith or numeric_std

    const bool isSigned = hif::typeIsSigned(in1, _sem) && hif::typeIsSigned(in2, _sem);

    const bool isBitType = dynamic_cast<Bit *>(top1) != nullptr && dynamic_cast<Bit *>(top2);

    std::string fName("hif_systemc_hif_logicEquals");
    if (_cLine.useHDTLib())
        fName += "_hdtlib";

    FunctionCall *fc = nullptr;
    if (!isBitType) {
        fc = _factory.functionCall(
            fName, _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
            _factory.noTemplateArguments(),
            (_factory.parameterArgument("param1", o->setValue1(nullptr)),
             _factory.parameterArgument("param2", o->setValue2(nullptr)),
             _factory.parameterArgument("param3", _factory.boolval(isSigned))));
    } else {
        fc = _factory.functionCall(
            fName, _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
            _factory.noTemplateArguments(),
            (_factory.parameterArgument("param1", o->setValue1(nullptr)),
             _factory.parameterArgument("param2", o->setValue2(nullptr))));
    }

    o->replace(fc);
    const bool negCond = (o->getOperator() == op_neq || o->getOperator() == op_case_neq);

    _addHifLibrary("systemc_hif_systemc_extensions");
    hif::backends::makeParametersAssignable(fc, _sem, true);

    hif::backends::addEventualCast(fc, hif::semantics::getSemanticType(fc, _sem), exprType);

    if (negCond) {
        Expression *e = new Expression();
        fc->replace(e);
        e->setOperator(op_bnot);
        e->setValue1(fc);
        e->acceptVisitor(*this);
    }

    delete o;
    return true;
}

bool PreRefine_utilityLibraries::_fixShiftOperator(Expression *o)
{
    if (!hif::operatorIsShift(o->getOperator()))
        return false;
    Type *t1 = hif::semantics::getSemanticType(o->getValue1(), _sem);
    messageAssert(t1 != nullptr, "Cannot type operand 1", o, _sem);
    Type *t2 = hif::semantics::getSemanticType(o->getValue2(), _sem);
    messageAssert(t2 != nullptr, "Cannot type operand 2", o, _sem);

    if (!hif::typeIsLogic(t2, _sem) || !hif::semantics::isVectorType(t1, _sem))
        return false;
    messageAssert(
        hif::typeIsLogic(t2, _sem) && hif::semantics::isVectorType(t2, _sem), "Required two logic vectors.", o, _sem);

    std::string funcName;
    if (o->getOperator() == op_sla || o->getOperator() == op_sll)
        funcName = "hif_systemc_hif_op_shift_left";
    else if (o->getOperator() == op_sra)
        funcName = "hif_systemc_hif_op_shift_right_arith";
    else if (o->getOperator() == op_srl)
        funcName = "hif_systemc_hif_op_shift_right_logic";
    else
        messageError("Unexpected case.", o, _sem);

    FunctionCall *fc = _factory.functionCall(
        funcName, _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
        _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", o->setValue1(nullptr)),
         _factory.parameterArgument("param2", o->setValue2(nullptr))));

    o->replace(fc);
    delete o;

    _addHifLibrary("systemc_hif_systemc_extensions");

    return true;
}

Value *PreRefine_utilityLibraries::_checkArrayOfSizeOneCase(Value *op, Array *opType, Array *otherOpType)
{
    if (opType == nullptr || otherOpType == nullptr)
        return nullptr;
    Cast *c = dynamic_cast<Cast *>(op);
    if (c == nullptr)
        return nullptr;
    Type *vt = hif::semantics::getSemanticType(c->getValue(), _sem);
    messageAssert(vt != nullptr, "Cannot type description", c->getValue(), _sem);
    Type *vtb = hif::semantics::getBaseType(vt, false, _sem, true);
    hif::EqualsOptions eqOpt;
    eqOpt.checkConstexprFlag = false;
    if (!hif::equals(vtb, otherOpType->getType(), eqOpt))
        return nullptr;
    Range *simplified                       = hif::manipulation::getAggressiveSimplified(opType->getSpan(), _sem);
    const unsigned long long int opSpanSize = hif::semantics::spanGetBitwidth(simplified, _sem);
    delete simplified;
    if (opSpanSize != 1ULL)
        return nullptr;

    return c->setValue(nullptr);
}

bool PreRefine_utilityLibraries::_fixArrayConcat(Expression *o)
{
    if (o->getOperator() != op_concat)
        return false;

    Type *t = hif::semantics::getSemanticType(o, _sem);
    messageAssert(t != nullptr, "Cannot type expression", o, _sem);
    Type *baseT = hif::semantics::getBaseType(t, false, _sem);
    messageAssert(baseT != nullptr, "Cannot find base type of expression", o, _sem);
    Type *t1 = hif::semantics::getSemanticType(o->getValue1());
    messageAssert(t1 != nullptr, "Cannot type operand 1", o, _sem);
    Type *base1 = hif::semantics::getBaseType(t1, false, _sem, true);
    messageAssert(base1 != nullptr, "Cannot find base type 1", o, _sem);
    Type *t2 = hif::semantics::getSemanticType(o->getValue2());
    messageAssert(t2 != nullptr, "Cannot type operand 2", o, _sem);
    Type *base2 = hif::semantics::getBaseType(t2, false, _sem, true);
    messageAssert(base2 != nullptr, "Cannot find base type 2", o, _sem);

    Array *ae = dynamic_cast<Array *>(baseT);
    Array *a1 = dynamic_cast<Array *>(base1);
    Array *a2 = dynamic_cast<Array *>(base2);

    if (a1 == nullptr && a2 == nullptr)
        return false;
    messageAssert(ae != nullptr, "Unexpected result type", baseT, _sem);

    // constructor call
    FunctionCall *constrCall = _factory.classConstructorCall(
        "instance",
        _factory.viewRef(
            "hif_systemc_ArrayConcat", "cpp",
            _factory.library("hif_systemc_hif_systemc_extensions", nullptr, nullptr, false, true),
            (_factory.templateTypeArgument("T", hif::copy(ae->getType())))),
        _factory.noParameterArguments(), _factory.noTemplateArguments());

    std::string concatName;

    Value *p1 = _checkArrayOfSizeOneCase(o->getValue1(), a1, a2);
    Value *p2 = _checkArrayOfSizeOneCase(o->getValue2(), a2, a1);

    const bool isArray1 = a1 != nullptr && p1 == nullptr;
    const bool isArray2 = a2 != nullptr && p2 == nullptr;

    if (isArray1 && isArray2)
        concatName = "concatArrays";
    else if (isArray1)
        concatName = "concatArrayWithValue";
    else /*(isArray2)*/
        concatName = "concatValueWithArray";

    FunctionCall *concatCall =
        _factory.functionCall(concatName, constrCall, _factory.noTemplateArguments(), _factory.noParameterArguments());

    if (p1 == nullptr)
        p1 = o->setValue1(nullptr);
    concatCall->parameterAssigns.push_back(_factory.parameterArgument("param1", p1));

    if (isArray1) {
        Value *s1 = hif::semantics::spanGetSize(a1->getSpan(), _sem);
        messageAssert(s1 != nullptr, "Cannot find type span size 1", o, _sem);
        concatCall->templateParameterAssigns.push_back(_factory.templateValueArgument("s1", s1));
    }

    if (p2 == nullptr)
        p2 = o->setValue2(nullptr);
    concatCall->parameterAssigns.push_back(_factory.parameterArgument("param2", p2));

    if (isArray2) {
        Value *s2 = hif::semantics::spanGetSize(a2->getSpan(), _sem);
        messageAssert(s2 != nullptr, "Cannot find type span size 2", o, _sem);
        concatCall->templateParameterAssigns.push_back(_factory.templateValueArgument("s2", s2));
    }

    _addHifLibrary("systemc_hif_systemc_extensions");

    o->replace(concatCall);
    hif::backends::addEventualCast(concatCall, hif::semantics::getSemanticType(concatCall, _sem), t);
    delete o;

    // Fix parameters etc
    concatCall->acceptVisitor(*this);

    return true;
}

void PreRefine_utilityLibraries::_manageReadCall(Value *o)
{
    // Fine also for hdtlib.

    Identifier *prefix = dynamic_cast<Identifier *>(hif::getTerminalPrefix(o));
    if (prefix == nullptr)
        return;

    FunctionCall *suspicioua = dynamic_cast<FunctionCall *>(prefix->getParent());
    if (suspicioua != nullptr && suspicioua->getInstance() == prefix && objectMatchName(suspicioua, "hif_systemc_read"))
        return;

    DataDeclaration *deco = hif::semantics::getDeclaration(prefix, _sem);
    messageAssert(deco != nullptr, "Declaration not found.", prefix, _sem);

    // Add .read() function if the source of assignment is a Port or a Signal.
    if (dynamic_cast<Port *>(deco) == nullptr && dynamic_cast<Signal *>(deco) == nullptr)
        return;

    // Skip use of port variables in sensitivity list.
    ObjectSensitivityOptions opts;
    opts.directParent = true;
    if (hif::objectIsInSensitivityList(o, opts))
        return;

    // Skip use of port variables in port assign
    if (dynamic_cast<PortAssign *>(hif::getParentSkippingCasts(o)) != nullptr)
        return;

    // Skip use of port variables in wait sensitivity list
    if (dynamic_cast<Wait *>(o->getParent()) != nullptr) {
        Wait *w = static_cast<Wait *>(o->getParent());
        if (o->getBList() == reinterpret_cast<BList<Object> *>(&w->sensitivity))
            return;
    }

    // Check if this is the target of assignment.
    if (hif::manipulation::isInLeftHandSide(o))
        return;

    // Skip when is used in a special standard call
    {
        // 1- parameter
        ParameterAssign *pa = dynamic_cast<ParameterAssign *>(o->getParent());
        FunctionCall *fc    = (pa != nullptr) ? dynamic_cast<FunctionCall *>(pa->getParent()) : nullptr;
        ProcedureCall *pc   = (pa != nullptr) ? dynamic_cast<ProcedureCall *>(pa->getParent()) : nullptr;

        if (fc != nullptr) {
            if (fc->getName() == "hif_systemc_hif_lastValue")
                return;
        }

        if (pc != nullptr) {
            if (pc->getName() == "hif_systemc_hif_assign")
                return;
            if (pc->getName() == "hif_systemc_hif_lastValue")
                return;
        }
    }

    if (_isInstanceOfStandardMethod(o))
        return;

    // In case that the value is still an array, it means a multi-dimensional
    // array of signals: we cannot add .read().
    // E.g. sign< bit[][] > s; s[6] <-- no read!
    // Reference design: verilog openCores can_top
    {
        Type *valueType = hif::semantics::getSemanticType(o, _sem);
        messageAssert(valueType != nullptr, "Cannot type value", o, _sem);
        Array *arr = dynamic_cast<Array *>(hif::semantics::getBaseType(valueType, false, _sem));
        if (arr != nullptr)
            return;
    }

    // In case of an Identifier which is prefix of a Member, two possibilities:
    // - we are interested on the i-th bit read: "var.read()[i]", the Identifier
    // can be replaced here.
    // - we are interested on the i-th element read: "var[i].read()", the entire
    // Member must be replaced.
    Array *arr = dynamic_cast<Array *>(hif::semantics::getBaseType(deco->getType(), false, _sem));
    if (arr != nullptr && prefix == o) {
        return;
    }

    // Else, replace the identifier only.
    FunctionCall *fCall = new FunctionCall();
    fCall->setName("hif_systemc_read");
    o->replace(fCall);
    fCall->setInstance(o);

    _addHifLibrary("systemc_sc_core");
}

bool PreRefine_utilityLibraries::_fixSlice(Slice *o)
{
    // fine for both hdtlib and sysc.
    Type *t     = hif::semantics::getSemanticType(o, _sem);
    Type *baseT = hif::semantics::getBaseType(t, false, _sem);
    messageAssert(t != nullptr, "Cannot type slice", o, _sem);

    if (hif::manipulation::isInLeftHandSide(o))
        return false;
    ObjectSensitivityOptions opts;
    opts.directParent = true;
    if (hif::objectIsInSensitivityList(o, opts))
        return false;
    if (_isInstanceOfStandardMethod(o))
        return false;
    if (dynamic_cast<Array *>(baseT) != nullptr)
        return false; // range on Array is managed through hif_range()

    FunctionCall *fCall      = nullptr;
    const bool isStringSlice = (dynamic_cast<String *>(baseT) != nullptr);
    if (isStringSlice) {
        Value *v = hif::semantics::spanGetSize(o->getSpan(), _sem);
        fCall    = _factory.functionCall(
            "hif_systemc_substr", o->setPrefix(nullptr), _factory.noTemplateArguments(),
            (_factory.parameterArgument(
                 "param1",
                 hif::manipulation::assureSyntacticType(hif::copy(hif::rangeGetMinBound(o->getSpan())), _sem)),
             _factory.parameterArgument("param2", hif::manipulation::assureSyntacticType(v, _sem))));
        _addHifLibrary("systemc_string");
    } else if (_cLine.useHDTLib()) {
        Value *v = hif::semantics::spanGetSize(o->getSpan(), _sem);
        fCall    = _factory.functionCall(
            "hif_systemc_range", o->setPrefix(nullptr), _factory.templateValueArgument("W", v),
            (_factory.parameterArgument(
                 "param1", hif::manipulation::assureSyntacticType(o->getSpan()->setLeftBound(nullptr), _sem)),
             _factory.parameterArgument(
                 "param2", hif::manipulation::assureSyntacticType(o->getSpan()->setRightBound(nullptr), _sem))));
        _addHifLibrary("systemc_hdtlib");
    } else {
        fCall = _factory.functionCall(
            "hif_systemc_range", o->setPrefix(nullptr), _factory.noTemplateArguments(),
            (_factory.parameterArgument(
                 "param1", hif::manipulation::assureSyntacticType(o->getSpan()->setLeftBound(nullptr), _sem)),
             _factory.parameterArgument(
                 "param2", hif::manipulation::assureSyntacticType(o->getSpan()->setRightBound(nullptr), _sem))));
        _addHifLibrary("systemc_sc_core");
    }

    o->replace(fCall);

    hif::backends::makeParametersAssignable(fCall, _sem, true);

    hif::backends::addEventualCast(fCall, hif::semantics::getSemanticType(fCall, _sem), t);
    delete o;

    // Fix parameters etc
    fCall->acceptVisitor(*this);

    return true;
}

bool PreRefine_utilityLibraries::_fixArraySlice(Slice *o)
{
    // fine for both hdtlib and sysc.
    Type *t     = hif::semantics::getSemanticType(o, _sem);
    Type *baseT = hif::semantics::getBaseType(t, false, _sem);
    messageAssert(t != nullptr, "Cannot type slice", o, _sem);

    if (hif::manipulation::isInLeftHandSide(o))
        return false;
    ObjectSensitivityOptions opts;
    opts.directParent = true;
    if (hif::objectIsInSensitivityList(o, opts))
        return false;
    if (_isInstanceOfStandardMethod(o))
        return false;
    Array *arr = dynamic_cast<Array *>(baseT);
    if (arr == nullptr)
        return false;

    Value *param2 = hif::copy(hif::rangeGetMaxBound(arr->getSpan()));
    hif::manipulation::assureSyntacticType(param2, _sem);
    Value *param3 = hif::copy(hif::rangeGetMinBound(arr->getSpan()));
    hif::manipulation::assureSyntacticType(param3, _sem);
    FunctionCall *fc = _factory.functionCall(
        "hif_systemc_hif_vector_slice", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
        _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", o->setPrefix(nullptr)), _factory.parameterArgument("param2", param2),
         _factory.parameterArgument("param3", param3)));

    _addHifLibrary("systemc_hif_systemc_extensions");

    o->replace(fc);
    hif::backends::makeParametersAssignable(fc, _sem, true);
    hif::backends::addEventualCast(fc, hif::semantics::getSemanticType(fc, _sem), t);
    delete o;

    return true;
}

void PreRefine_utilityLibraries::_setAssignBounds(Value *target, ProcedureCall *ret, const char recursion)
{
    // At the moment, only bi-dimensional array are managed.
    messageAssert(recursion <= '3', "Array with more than 2 dimension are not supported yet", target, _sem);

    // TODO check
    // If Member, 'left' and 'right' parameter are assigned by default.

    if (dynamic_cast<Member *>(target) != nullptr) {
        //        Member* mm = static_cast< Member* >( target );
        //        assert( mm->value.size() == 1 );
        //
        //        ParameterAssign * left = new ParameterAssign();
        //        std::string leftName = std::string("left") + recursion;
        //        left->setName(  leftName.c_str( ) );
        //        left->setValue( hif::copy( mm->value.front()));
        //        hif::manipulation::assureSyntacticType(left->getValue(), _sem);
        //
        //        ParameterAssign * right = new ParameterAssign();
        //        std::string rightName = std::string("right") + recursion;
        //        right->setName(  rightName.c_str( ) );
        //        right->setValue( hif::copy( mm->value.front()));
        //        hif::manipulation::assureSyntacticType(right->getValue(), _sem);
        //
        //        _setAssignBounds( mm->getPrefix(), ret, static_cast<char>(recursion+1) );
        //
        //        ret->parameterAssigns.push_back(left);
        //        ret->parameterAssigns.push_back(right);
    } else if (dynamic_cast<Slice *>(target) != nullptr) {
        Slice *ss = static_cast<Slice *>(target);

        ParameterAssign *left = new ParameterAssign();
        std::string leftName  = std::string("left") + recursion;
        left->setName(leftName);
        left->setValue(hif::copy(ss->getSpan()->getLeftBound()));
        hif::manipulation::assureSyntacticType(left->getValue(), _sem);

        ParameterAssign *right = new ParameterAssign();
        std::string rightName  = std::string("right") + recursion;
        right->setName(rightName);
        right->setValue(hif::copy(ss->getSpan()->getRightBound()));
        hif::manipulation::assureSyntacticType(right->getValue(), _sem);

        _setAssignBounds(ss->getPrefix(), ret, static_cast<char>(recursion + 1));

        ret->parameterAssigns.push_back(left);
        ret->parameterAssigns.push_back(right);
    }
}

ProcedureCall *PreRefine_utilityLibraries::_makeStandardAssignProcedure(Value *target, Value *sourceValue)
{
    Type *srcType = hif::semantics::getBaseType(hif::semantics::getSemanticType(sourceValue, _sem), false, _sem);

    messageAssert(srcType != nullptr, "Cannot calculate base type", sourceValue, _sem);

    const unsigned int srcCard = hif::typeGetCardinality(srcType, _sem, false);

    Value *tgtNarrow = hif::manipulation::narrowToCardinality(target, srcCard, _sem, false);
    if (tgtNarrow == nullptr) {
        stringstream ss;
        ss << "Cannot calculate narrowToCardinality with cardinality=" << srcCard;
        messageDebug("Source value:", sourceValue, _sem);
        messageError(ss.str(), target, _sem);
    }

    Value *var = tgtNarrow;
    while (dynamic_cast<Slice *>(var) != nullptr) {
        Slice *tmp                   = static_cast<Slice *>(var);
        unsigned long long sliceSize = hif::semantics::spanGetBitwidth(tmp->getSpan(), _sem);
        if (sliceSize != 1ull) {
            var = tmp->getPrefix();
        } else {
            break;
        }
    }

    // creating target param
    ParameterAssign *p1 = new ParameterAssign();
    p1->setName("target");
    p1->setValue(hif::copy(var));

    // creating source param
    ParameterAssign *p2 = new ParameterAssign();
    p2->setName("source");
    p2->setValue(hif::copy(sourceValue));

    // creating size param
    Type *tgtType   = hif::semantics::getBaseType(hif::semantics::getSemanticType(target, _sem), false, _sem);
    Range *tgtTspan = hif::typeGetSpan(tgtType, _sem);

    ParameterAssign *size = new ParameterAssign();
    size->setName("size");
    size->setValue(hif::semantics::spanGetSize(tgtTspan, _sem));

    Cast *c = new Cast();
    c->setValue(size->setValue(nullptr));
    c->setType(_factory.integer(nullptr, false, true));
    size->setValue(c);

    ProcedureCall *pc = new ProcedureCall();
    pc->setName("hif_systemc_hif_assign");
    pc->setInstance(_factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true));
    pc->parameterAssigns.push_back(p1);
    pc->parameterAssigns.push_back(p2);
    pc->parameterAssigns.push_back(size);

    return pc;
}

Value *PreRefine_utilityLibraries::_getHifAssignSource(Value *source)
{
    while (dynamic_cast<Cast *>(source) != nullptr) {
        Cast *co = static_cast<Cast *>(source);

        Type *coT = hif::semantics::getBaseType(co->getType(), false, _sem);
        Type *opT = hif::semantics::getBaseType(hif::semantics::getSemanticType(co->getValue(), _sem), false, _sem);

        Array *coTa = dynamic_cast<Array *>(coT);
        Array *opTa = dynamic_cast<Array *>(opT);

        Bitvector *coTbv = dynamic_cast<Bitvector *>(coT);
        Bitvector *opTbv = dynamic_cast<Bitvector *>(opT);

        Signed *coTs = dynamic_cast<Signed *>(coT);
        Signed *opTs = dynamic_cast<Signed *>(opT);

        Unsigned *coTus = dynamic_cast<Unsigned *>(coT);
        Unsigned *opTus = dynamic_cast<Unsigned *>(opT);

        Int *coTi = dynamic_cast<Int *>(coT);
        Int *opTi = dynamic_cast<Int *>(opT);

        if ((coTa == nullptr && coTbv == nullptr && coTs == nullptr && coTus == nullptr && coTi == nullptr) ||
            (opTa == nullptr && opTbv == nullptr && opTs == nullptr && opTus == nullptr && opTi == nullptr))
            break;

        Value *spanT           = hif::semantics::spanGetSize(hif::typeGetSpan(coT, _sem), _sem);
        Value *spanOp          = hif::semantics::spanGetSize(hif::typeGetSpan(opT, _sem), _sem);
        const bool cmp         = hif::equals(spanT, spanOp);
        const bool castToArray = coTa && !opTa;
        delete spanT;
        delete spanOp;
        if (!cmp && !castToArray)
            break;

        source = co->getValue();
    }

    // Checking if the cast op is an integer casted to array.
    // In this case, it means that the op was an array/bitvector moved to template.
    Cast *c = dynamic_cast<Cast *>(source);
    if (c == nullptr)
        return source;

    Type *coT = hif::semantics::getBaseType(c->getType(), false, _sem);
    Type *opT = hif::semantics::getBaseType(hif::semantics::getSemanticType(c->getValue(), _sem), false, _sem);

    Array *coTa = dynamic_cast<Array *>(coT);
    Int *opTi   = dynamic_cast<Int *>(opT);

    if (coTa == nullptr || opTi == nullptr)
        return source;

    Bit *b = dynamic_cast<Bit *>(coTa->getType());
    if (b == nullptr)
        return source;

    Bitvector *bv = new Bitvector();
    bv->setLogic(b->isLogic());
    bv->setResolved(b->isResolved());
    bv->setSigned(coTa->isSigned());
    bv->setConstexpr(typeIsConstexpr(coTa, _sem));
    bv->setSpan(hif::copy(coTa->getSpan()));

    delete c->setType(bv);
    hif::semantics::resetTypes(c, false);

    return source;
}

bool PreRefine_utilityLibraries::_manageAssignToSlice(Assign &o)
{
    if (_cLine.useHDTLib())
        return false;

    if (dynamic_cast<Slice *>(o.getLeftHandSide()) == nullptr)
        return false;
    Slice *tgt = static_cast<Slice *>(o.getLeftHandSide());

    Identifier *id = dynamic_cast<Identifier *>(hif::getTerminalPrefix(tgt));
    if (id == nullptr)
        return false;

    Declaration *idDec = hif::semantics::getDeclaration(id, hif::semantics::SystemCSemantics::getInstance());

    if (dynamic_cast<Signal *>(idDec) == nullptr && dynamic_cast<Port *>(idDec) == nullptr)
        return false;

    // A sort of getChildSkippingCast, but preserving eventual casts added
    // for sign manipulation. Thus, remove them if dimension is the same.
    Value *source = _getHifAssignSource(o.getRightHandSide());

    Type *t2 = hif::semantics::getBaseType(hif::semantics::getSemanticType(source, _sem), false, _sem);

    Array *t2a      = dynamic_cast<Array *>(t2);
    Bitvector *t2bv = dynamic_cast<Bitvector *>(t2);
    Signed *t2s     = dynamic_cast<Signed *>(t2);
    Unsigned *t2us  = dynamic_cast<Unsigned *>(t2);
    Int *t2i        = dynamic_cast<Int *>(t2);

    if (t2a == nullptr && t2bv == nullptr && t2s == nullptr && t2us == nullptr && t2i == nullptr) {
        messageDebug("Source of assign is ", source, _sem);
        messageError("Unxpected case", &o, nullptr);
    }

    ProcedureCall *pc = _makeStandardAssignProcedure(tgt, source);

    ParameterAssign *left = new ParameterAssign();
    left->setName("left1");
    left->setValue(hif::copy(tgt->getSpan()->getLeftBound()));
    hif::manipulation::assureSyntacticType(left->getValue(), _sem);

    ParameterAssign *right = new ParameterAssign();
    right->setName("right1");
    right->setValue(hif::copy(tgt->getSpan()->getRightBound()));
    hif::manipulation::assureSyntacticType(right->getValue(), _sem);

    pc->parameterAssigns.push_back(left);
    pc->parameterAssigns.push_back(right);

    o.replace(pc);

    // In case of Aggregate containing only 'others' the procedure call would
    // be no more typeable. Therefore we make it typeable by inserting an explicit
    // cast to its old type.
    ParameterAssign *p2 = pc->parameterAssigns.at(1);
    messageAssert(p2 != nullptr, "Expected param at pos 2", pc, _sem);
    Type *sourceT = hif::semantics::getSemanticType(p2->getValue(), _sem);
    if (sourceT == nullptr) {
        Bitvector *arr = new Bitvector();
        arr->setSpan(hif::copy(hif::typeGetSpan(t2, _sem)));
        arr->setConstexpr(hif::typeIsConstexpr(t2, _sem));
        arr->setSigned(hif::typeIsSigned(t2, _sem));
        arr->setResolved(hif::typeIsResolved(t2, _sem));
        arr->setLogic(hif::typeIsLogic(t2, _sem));

        Cast *co = new Cast();
        co->setType(arr);
        co->setValue(p2->getValue());
        p2->setValue(co);
    }
    delete &o;

    _addHifLibrary("systemc_hif_systemc_extensions");

    hif::backends::makeParametersAssignable(pc, _sem, true, 1);
    //pc->acceptVisitor(*this);

    return true;
}

bool PreRefine_utilityLibraries::_manageHdtlibSliceAssign(Assign *o)
{
    if (!_cLine.useHDTLib())
        return false;

    Slice *s = dynamic_cast<Slice *>(o->getLeftHandSide());
    if (s == nullptr)
        return false;

    Value *prefix = s->getPrefix();
    Type *t       = hif::semantics::getBaseType(hif::semantics::getSemanticType(prefix, _sem), false, _sem);
    if (dynamic_cast<Array *>(t) != nullptr)
        return false;

    Value *l = s->getSpan()->setLeftBound(nullptr);
    hif::manipulation::assureSyntacticType(l, _sem);

    Value *r = s->getSpan()->setRightBound(nullptr);
    hif::manipulation::assureSyntacticType(r, _sem);

    ProcedureCall *fCall = _factory.procedureCall(
        "hif_systemc_set_range", s->setPrefix(nullptr), _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", l), _factory.parameterArgument("param2", r),
         _factory.parameterArgument("param3", o->setRightHandSide(nullptr))));

    o->replace(fCall);
    delete o;

    _addHifLibrary("systemc_hdtlib");
    hif::backends::makeParametersAssignable(fCall, _sem, true);

    // Fix parameters etc
    fCall->acceptVisitor(*this);

    return true;
}

bool PreRefine_utilityLibraries::_manageHdtlibMemberAssign(Assign *o)
{
    if (!_cLine.useHDTLib())
        return false;

    Member *m = dynamic_cast<Member *>(o->getLeftHandSide());
    if (m == nullptr)
        return false;

    Value *prefix = m->getPrefix();
    Type *t       = hif::semantics::getBaseType(hif::semantics::getSemanticType(prefix, _sem), false, _sem);
    if (!hif::semantics::isVectorType(t, _sem))
        return false;

    Value *i = m->getIndex();
    i->replace(nullptr);
    hif::manipulation::assureSyntacticType(i, _sem);

    ProcedureCall *fCall = _factory.procedureCall(
        "hif_systemc_set_bit", m->setPrefix(nullptr), _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", i), _factory.parameterArgument("param2", o->setRightHandSide(nullptr))));

    o->replace(fCall);
    delete o;

    _addHifLibrary("systemc_hdtlib");
    hif::backends::makeParametersAssignable(fCall, _sem, true);

    // Fix parameters etc
    fCall->acceptVisitor(*this);

    return true;
}

bool PreRefine_utilityLibraries::_manageAssignBetweenArrays(Assign &o)
{
    // Fine also for hdtlib. TODO check

    // A sort of getChildSkippingCast, but preserving eventual casts added
    // for sign manipulation. Thus, remove them if dimension is the same.
    Value *source = _getHifAssignSource(o.getRightHandSide());

    Type *t1 = hif::semantics::getBaseType(hif::semantics::getSemanticType(o.getLeftHandSide(), _sem), false, _sem);

    Type *t2 = hif::semantics::getBaseType(hif::semantics::getSemanticType(source, _sem), false, _sem);

    Array *t1a = dynamic_cast<Array *>(t1);
    Array *t2a = dynamic_cast<Array *>(t2);

    Bitvector *t1bv = dynamic_cast<Bitvector *>(t1);
    Bitvector *t2bv = dynamic_cast<Bitvector *>(t2);

    Signed *t1s = dynamic_cast<Signed *>(t1);
    Signed *t2s = dynamic_cast<Signed *>(t2);

    Unsigned *t1us = dynamic_cast<Unsigned *>(t1);
    Unsigned *t2us = dynamic_cast<Unsigned *>(t2);

    Int *t1i = dynamic_cast<Int *>(t1);
    Int *t2i = dynamic_cast<Int *>(t2);

    if ((t1a == nullptr && t1bv == nullptr && t1s == nullptr && t1us == nullptr && t1i == nullptr) ||
        (t2a == nullptr && t2bv == nullptr && t2s == nullptr && t2us == nullptr && t2i == nullptr))
        return false;

    // if both are bit vector, signed, unsigned not need to call hif2sc_assign
    if (t1a == nullptr && t2a == nullptr)
        return false;

    ProcedureCall *pc = _makeStandardAssignProcedure(o.getLeftHandSide(), source);

    _setAssignBounds(o.getLeftHandSide(), pc, '1');

    o.replace(pc);

    // In case of Aggregate containing only 'others' the procedure call would
    // be no more typeable. Therefore we make it typeable by inserting an explicit
    // cast to its old type.
    ParameterAssign *p2 = pc->parameterAssigns.at(1);
    messageAssert(p2 != nullptr, "Expected param at pos 2", pc, _sem);
    Type *sourceT = hif::semantics::getSemanticType(p2->getValue(), _sem);
    if (sourceT == nullptr) {
        Bitvector *arr = new Bitvector();
        arr->setSpan(hif::copy(hif::typeGetSpan(t2, _sem)));
        arr->setConstexpr(hif::typeIsConstexpr(t2, _sem));
        arr->setSigned(hif::typeIsSigned(t2, _sem));
        arr->setResolved(hif::typeIsResolved(t2, _sem));
        arr->setLogic(hif::typeIsLogic(t2, _sem));

        Cast *co = new Cast();
        co->setType(arr);
        co->setValue(p2->getValue());
        p2->setValue(co);
    }

    _addHifLibrary("systemc_hif_systemc_extensions");

    delete &o;

    hif::backends::makeParametersAssignable(pc, _sem, true, 1);
    pc->acceptVisitor(*this);
    return true;
}

bool PreRefine_utilityLibraries::_manageAssignToStringSlice(Assign *o)
{
    Slice *leftSlice = dynamic_cast<Slice *>(o->getLeftHandSide());
    if (leftSlice == nullptr)
        return false;

    Type *leftType = hif::semantics::getSemanticType(leftSlice, _sem);
    messageAssert(leftType != nullptr, "Cannot type slice", leftSlice, _sem);

    Type *leftBaseType = hif::semantics::getBaseType(leftType, false, _sem, false);
    String *leftString = dynamic_cast<String *>(leftBaseType);
    if (leftString == nullptr)
        return false;

    Value *size = hif::semantics::spanGetSize(leftSlice->getSpan(), _sem);

    Value *pos = hif::manipulation::assureSyntacticType(leftSlice->getSpan()->setLeftBound(nullptr), _sem);

    ProcedureCall *pc = _factory.procedureCall(
        "hif_systemc_replace", leftSlice->setPrefix(nullptr), _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", pos), _factory.parameterArgument("param2", size),
         _factory.parameterArgument("param3", o->setRightHandSide(nullptr))));
    o->replace(pc);
    delete o;

    _addHifLibrary("systemc_string");

    hif::backends::makeParametersAssignable(pc, _sem, true);
    pc->acceptVisitor(*this);

    return true;
}

DataDeclaration *PreRefine_utilityLibraries::_getAggregateParent(Aggregate *o)
{
    Object *parent = o->getParent();
    while (dynamic_cast<Aggregate *>(parent) != nullptr || dynamic_cast<AggregateAlt *>(parent) != nullptr ||
           dynamic_cast<Cast *>(parent) != nullptr) {
        parent = parent->getParent();
    }

    return dynamic_cast<DataDeclaration *>(parent);
}

bool PreRefine_utilityLibraries::_manageAggregateArray(Aggregate *o)
{
    // Fine also for hdtlib.

    Type *oType    = hif::semantics::getSemanticType(o, _sem);
    Type *bt       = hif::semantics::getBaseType(oType, false, _sem, false);
    Array *arr     = dynamic_cast<Array *>(bt);
    Bitvector *bv  = dynamic_cast<Bitvector *>(bt);
    Signed *sig    = dynamic_cast<Signed *>(bt);
    Unsigned *usig = dynamic_cast<Unsigned *>(bt);
    if (arr == nullptr && bv == nullptr && sig == nullptr && usig == nullptr)
        return false;

    std::string callName;
    if (arr != nullptr) {
        DataDeclaration *ddecl = _getAggregateParent(o);
        if (ddecl != nullptr && hif::isSubNode(o, ddecl->getValue()))
            return false;

        callName = "hif_systemc_HifAggregateArray";
    } else if ((bv != nullptr && bv->isLogic()) || (sig != nullptr) || (usig != nullptr)) {
        if (_cLine.useHDTLib())
            callName = "hif_systemc_HifAggregateHlLv";
        else
            callName = "hif_systemc_HifAggregateLogicVector";
    } else if (bv != nullptr && !bv->isLogic()) {
        if (_cLine.useHDTLib())
            callName = "hif_systemc_HifAggregateHlBv";
        else
            callName = "hif_systemc_HifAggregateBitVector";
    }
    FunctionCall *fcall = _createAggregateCall(callName, bt, o);

    // In case of logic vector, signed and usigned types is necessary a cast
    // of original type to keep typing because these type can be assigned each other.
    // Otherwise cast should be useless.
    if ((bv != nullptr && bv->isLogic()) || sig != nullptr || usig != nullptr) {
        Cast *c = new Cast();
        c->setValue(fcall);
        c->setType(hif::copy(oType));
        o->replace(c);
    } else {
        o->replace(fcall);
    }

    delete o;

    _addHifLibrary("systemc_hif_systemc_extensions");
    if (_cLine.useHDTLib())
        _addHifLibrary("systemc_hdtlib");

    return true;
}

FunctionCall *PreRefine_utilityLibraries::_createAggregateCall(const std::string &callName, Type *t, Aggregate *agg)
{
    FunctionCall *mainCall = nullptr; // pointer to the most extern call

    // HifAggregateArray()
    Value *sizeParam = hif::semantics::spanGetSize(hif::typeGetSpan(t, _sem), _sem); // fresh

    const bool isArray = (dynamic_cast<Array *>(t) != nullptr);

    // constructor call
    if (isArray) {
        Array *tArr = static_cast<Array *>(t);
        mainCall    = _factory.classConstructorCall(
            "instance",
            _factory.viewRef(
                callName, "cpp", _factory.library("hif_systemc_hif_systemc_extensions", nullptr, nullptr, false, true),
                (_factory.templateTypeArgument("T", hif::copy(tArr->getType())),
                 _factory.templateValueArgument("size", sizeParam))),
            _factory.noParameterArguments(), _factory.noTemplateArguments());
    } else {
        mainCall = _factory.classConstructorCall(
            "instance",
            _factory.viewRef(
                callName, "cpp", _factory.library("hif_systemc_hif_systemc_extensions", nullptr, nullptr, false, true),
                (_factory.templateValueArgument("size", sizeParam))),
            _factory.noParameterArguments(), _factory.noTemplateArguments());
    }

    // eventually cast parameter assign (to assure assignability)
    Type *castType = nullptr;
    if (!isArray) {
        Bitvector *bv = dynamic_cast<Bitvector *>(t);
        if (bv != nullptr && !bv->isLogic())
            castType = _factory.boolean();
        else
            castType = _factory.bit(true, false);
    }

    // eventual setOthers()
    if (agg->getOthers() != nullptr) {
        Value *val = agg->setOthers(nullptr);
        if (castType != nullptr)
            val = _factory.cast(hif::copy(castType), val);

        mainCall = _factory.functionCall(
            "setOthers", mainCall, _factory.noTemplateArguments(), _factory.parameterArgument("others", val));
    }

    // setPair(), setPairSet()
    for (BList<AggregateAlt>::iterator it = agg->alts.begin(); it != agg->alts.end(); ++it) {
        for (BList<Value>::iterator jt = (*it)->indices.begin(); jt != (*it)->indices.end(); ++jt) {
            if (dynamic_cast<Range *>(*jt) != nullptr) {
                Range *r = static_cast<Range *>(*jt);

                Value *val = hif::copy((*it)->getValue());
                if (castType != nullptr)
                    val = _factory.cast(hif::copy(castType), val);

                mainCall = _factory.functionCall(
                    "addPairSet", mainCall, _factory.noTemplateArguments(),
                    (_factory.parameterArgument(
                         "lbound", hif::manipulation::assureSyntacticType(r->setLeftBound(nullptr), _sem)),
                     _factory.parameterArgument(
                         "rbound", hif::manipulation::assureSyntacticType(r->setRightBound(nullptr), _sem)),
                     _factory.parameterArgument("value", val)));
            } else {
                Value *val = hif::copy((*it)->getValue());
                if (castType != nullptr)
                    val = _factory.cast(hif::copy(castType), val);

                mainCall = _factory.functionCall(
                    "addPair", mainCall, _factory.noTemplateArguments(),
                    (_factory.parameterArgument("index", hif::manipulation::assureSyntacticType(hif::copy(*jt), _sem)),
                     _factory.parameterArgument("value", val)));
            }
        }
    }

    // getResult()
    mainCall =
        _factory.functionCall("getResult", mainCall, _factory.noTemplateArguments(), _factory.noParameterArguments());

    delete castType;
    return mainCall;
}

bool PreRefine_utilityLibraries::_fixAfter(Assign *o)
{
    if (o->getDelay() == nullptr)
        return false;

    Type *targetType = hif::semantics::getSemanticType(o->getLeftHandSide(), _sem);
    messageAssert(targetType != nullptr, "Cannot type target", o->getLeftHandSide(), _sem);

    // Always creating support var to allow assignments between arrays, slices, etc.
    // TODO Issue: target of type array

    ProcedureCall *pcall = _factory.procedureCall(
        "hif_systemc_hif_after", _factory.libraryInstance("hif_systemc_hif_systemc_extensions", false, true),
        _factory.noTemplateArguments(),
        (_factory.parameterArgument("param1", o->setLeftHandSide(nullptr)),
         _factory.parameterArgument("param2", _factory.cast(hif::copy(targetType), o->setRightHandSide(nullptr))),
         _factory.parameterArgument("param3", o->setDelay(nullptr))));

    o->replace(pcall);
    _trash.insert(o);

    _addHifLibrary("systemc_hif_systemc_extensions");

    raiseUniqueWarning("Found at least one delayed assignment. Generated SystemC code must "
                       "allow multiple "
                       "drivers to be run (i.e.: export SC_SIGNAL_WRITE_CHECK=DISABLE).");

    return true;
}

void PreRefine_utilityLibraries::_addHifLibrary(const std::string &c, const bool standard)
{
    std::string nn("hif_");
    nn += c;

    _introducedLibraries |= hif::backends::addHifLibrary(nn, _scope, _root, _sem, standard);
}

template <typename T> bool PreRefine_utilityLibraries::_fixFunctionWithPedix(T *o)
{
    typename T::DeclarationType *decl = hif::semantics::getDeclaration(o, _sem);
    messageAssert(decl != nullptr, "Declaration not found", o, _sem);

    LibraryDef *ld = dynamic_cast<LibraryDef *>(decl->getParent());
    if (ld == nullptr || !ld->isStandard())
        return false;

    std::string pedix(decl->getName());
    if (_cLine.useHDTLib() && _inHDTLibRelatedLibrary(ld->getName(), decl->getName())) {
        pedix += "_hdtlib";
    } else if (_inSignedUnsignedRelatedLibrary(ld->getName(), decl->getName())) {
        if (decl->parameters.empty())
            return false;
        Bit *b = dynamic_cast<Bit *>(decl->parameters.front()->getType());
        if (b != nullptr)
            return false;
        if (hif::typeIsSigned(decl->parameters.front()->getType(), _sem)) {
            pedix += "_signed";
        } else {
            pedix += "_unsigned";
        }
    } else {
        return false;
    }

    o->setName(pedix);
    return true;
}

template <typename T> bool PreRefine_utilityLibraries::_fixResolvedMethods(T *o)
{
    typename T::DeclarationType *decl = hif::semantics::getDeclaration(o, _sem);
    messageAssert(decl != nullptr, "Declaration not found", o, _sem);

    LibraryDef *ld = dynamic_cast<LibraryDef *>(decl->getParent());
    if (ld == nullptr || !ld->isStandard())
        return false;

    SubProgram *resolvedSub   = nullptr;
    SubProgram *unresolvedSub = nullptr;
    if (_isResolvedConflicting(ld->getName(), decl, resolvedSub, unresolvedSub)) {
        _trash.insert(resolvedSub);
        hif::semantics::resetDeclarations(o);
        hif::semantics::resetTypes(o);
        hif::semantics::setDeclaration(o, unresolvedSub);
        hif::backends::makeParametersAssignable(o, _sem, true);

        return true;
    }

    return false;
}

bool PreRefine_utilityLibraries::_isResolvedConflicting(
    const std::string &lib,
    SubProgram *decl,
    SubProgram *&resolvedSub,
    SubProgram *&unresolvedSub)
{
    // Ref designs for all follwing cases:
    // - vhdl/openCores/plasma

    if (lib == "hif_vhdl_ieee_std_logic_1164") {
        auto func = decl->getName();
        if (func == "hif_vhdl_is_x") {
            if (dynamic_cast<Bit *>(decl->parameters.front()->getType()) != nullptr) {
                return false;
            } else if (hif::typeIsResolved(decl->parameters.front()->getType(), _sem)) {
                resolvedSub = decl;
                unresolvedSub =
                    _getResolvedUnresolvedSubprogram(dynamic_cast<LibraryDef *>(decl->getParent()), func, 0, false);
            } else {
                unresolvedSub = decl;
                resolvedSub =
                    _getResolvedUnresolvedSubprogram(dynamic_cast<LibraryDef *>(decl->getParent()), func, 0, true);
                messageAssert(resolvedSub != nullptr, "Unexpected declaration", resolvedSub, _sem);
            }
            return true;
        }
    } else if (lib == "hif_vhdl_ieee_std_logic_textio") {
        auto func = decl->getName();
        if (func == "hif_vhdl_hread") {
            if (dynamic_cast<Bit *>(decl->parameters.at(1)->getType()) != nullptr) {
                return false;
            } else if (hif::typeIsResolved(decl->parameters.at(1)->getType(), _sem)) {
                resolvedSub = decl;
                unresolvedSub =
                    _getResolvedUnresolvedSubprogram(dynamic_cast<LibraryDef *>(decl->getParent()), func, 1, false);
            } else {
                unresolvedSub = decl;
                resolvedSub =
                    _getResolvedUnresolvedSubprogram(dynamic_cast<LibraryDef *>(decl->getParent()), func, 1, true);
                messageAssert(resolvedSub != nullptr, "Unexpected declaration", resolvedSub, _sem);
            }
            return true;
        }
    }

    return false;
}

SubProgram *PreRefine_utilityLibraries::_getResolvedUnresolvedSubprogram(
    LibraryDef *ld,
    const std::string &subName,
    const BListHost::size_t pos,
    const bool resolved)
{
    if (ld == nullptr)
        return nullptr;
    for (BList<Declaration>::iterator i = ld->declarations.begin(); i != ld->declarations.end(); ++i) {
        Declaration *decl = *i;
        if (decl->getName() != subName)
            continue;
        SubProgram *sub = dynamic_cast<SubProgram *>(decl);
        if (sub == nullptr || sub->parameters.size() <= pos)
            continue;
        if (hif::typeIsResolved(sub->parameters.at(pos)->getType(), _sem) != resolved)
            continue;
        return sub;
    }

    return nullptr;
}

bool PreRefine_utilityLibraries::_inSignedUnsignedRelatedLibrary(const std::string &lib, const std::string &func)
{
    if (lib == "hif_vhdl_ieee_std_logic_arith") {
        if (func == "hif_vhdl_shr")
            return true;
        else if (func == "hif_vhdl_conv_integer")
            return true;
        else if (func == "hif_vhdl_conv_std_logic_vector")
            return true;
        else if (func == "hif_vhdl_conv_unsigned")
            return true;
        else if (func == "hif_vhdl_conv_signed")
            return true;
    } else if (lib == "hif_vhdl_ieee_numeric_std") {
        if (func == "hif_vhdl_to_integer")
            return true;
    }

    return false;
}

bool PreRefine_utilityLibraries::_inHDTLibRelatedLibrary(const std::string &lib, const std::string &func)
{
    if (lib == "hif_verilog_standard") {
        if (func == "hif_verilog__system_finish")
            return true;
        else if (func == "hif_verilog__system_stop")
            return true;
        else if (func == "hif_verilog__system_stime")
            return true;
        else if (func == "hif_verilog__system_time")
            return true;
    } else if (lib == "hif_systemc_extensions") {
        if (func == "hif_logicEquals")
            return true;
    }

    return false;
}

} // namespace

bool fixUtilityLibraries(hif::System *o, hif::semantics::ILanguageSemantics *sem, const hif2scParseLine &cLine)
{
    hif::application_utils::initializeLogHeader("HIF2SC", "fixUtilityLibraries");

    // Step 1: last_value

    const bool lastValueFix = hif::manipulation::mapLastValueToSystemC(o);

    hif::manipulation::AddUniqueObjectOptions addOpt;
    addOpt.equalsOptions.checkOnlyNames = true;
    addOpt.position                     = 0;

    LibraryDef *stdLib = sem->getStandardLibrary("hif_systemc_standard");
    hif::manipulation::addUniqueObject(stdLib, o->libraryDefs, addOpt);

    // Second step: other fixes
    hif::manipulation::RemoveStandardMethodsOptions ropts;
    ropts.preferScMethodFlavour = true;
    ropts.allowSystemCAMS       = true;
    hif::manipulation::removeStandardMethods(o, sem, ropts);
    PreRefine_utilityLibraries vis(o, sem, cLine);
    o->acceptVisitor(vis);

    // Third step: ensuring hif_systemc_extensions, since some HIF-to-SystemC casts need it.
    // Add systemc_extension library def.
    LibraryDef *extLib = sem->getStandardLibrary("hif_systemc_hif_systemc_extensions");
    hif::manipulation::addUniqueObject(extLib, o->libraryDefs, addOpt);

    hif::application_utils::restoreLogHeader();

    return (lastValueFix || vis.hasIntroducedLibraries());
}
