/// @file PrintSystemCVisitor.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

#include "hif2sc/globals.hpp"

/// @brief Options regulating the visit mode.
///
struct PrintSystemCVisitorOptions {
    /// @name Command-line flags.
    /// @{

    /// @brief Allows the print of resolved signals/ports. Default: false.
    bool useResolved;

    /// @brief Force the print of HDTLib types instead of SystemC types. Default: false.
    bool useHDTLib;

    /// @brief Prints to be compliant with Cpp98 compiler.
    bool useCpp98;

    /// @brief The maximum line numbers before splitting files.
    uint64_t maxLines;

    /// @}

    /// @name Information management.

    /// @brief Force the print of the visited-object type.
    bool printType;

    /// @brief Force the print of the visited-object initial value.
    bool printInitVal;

    /// @brief Distinguishes between calls from PrintHeaderVisitor (if false) and
    /// PrintImplementationVisitor (if true).
    bool printImplementation;

    /// @brief In case of call from PrintImplementation visitor, distinguishes
    /// between request for header implementation or source file.
    bool printImplementation_ihh;

    /// @brief In case of call from PrintHeaderVisitor visitor, distinguishes
    /// between private or public declarations.
    bool publicDecl;

    /// @brief Force the print of the visited-object initial value in
    /// constructor-initialization-list fashion.
    /// - Note: mutually exclusive with respect to flag insideConstructor.
    /// - Note: considered only if printImplementation is set true.
    bool insideInitList;

    /// @brief Force the print of the visited-object initial value in
    /// constructor-body.
    /// -Note: mutually exclusive with respect to flag insideInitList.
    /// -Note: considered only if printImplementation is set true.
    bool insideConstructorBody;

    /// @brief Identifies an empty initialization list.
    /// - Note: considered only if insideInitList is set true.
    bool emptyInitList;

    /// @brief This flag alter the normal print flow for Const, which can be printed
    /// outside their original HIF scope, and require a totally different management.
    /// If set to Const*, implies to consider the constant only, indipendently from scope.
    /// If set to View*, implies to consider the module's scope.
    hif::Declaration *constManagement;

    /// @}

    /// @name Visit of composite types.
    /// @{

    /// @brief  Flag used in VisitRecord. It distinguishes between the first call
    /// (print of "struct") and the second (print of record fields).
    bool printFields;

    /// @brief Flag used in VisitArray. An array of array, e.g.
    /// @code sc_signal< sc_lv<32> > sig[16];
    /// contains two distinct spans in two different locations, thus visit of
    /// array has to be called twice to print them correctly. This flag is used
    /// to understand when each one has to be printed.
    bool printSquareSpan;

    /// @brief  Flag used in VisitRecord. It distinguishes between the first call
    /// (print of "int") and the second (print of bit field ": 3").
    bool printBitFields;

    /// @brief Flag used in VisitCast. It force the printing of full array type.
    bool printFullType;

    /// @}

    std::string sourcesExtension;
    std::string headersExtension;

    PrintSystemCVisitorOptions();
    ~PrintSystemCVisitorOptions();

    PrintSystemCVisitorOptions(const PrintSystemCVisitorOptions &);
    auto operator=(const PrintSystemCVisitorOptions &) -> PrintSystemCVisitorOptions &;
};

/// @brief Visitor that actually prints the code. Basically 3 print modes can
/// be identified: modes 1 and 2 are characterized by flag printImplementation
/// set to false and managed through flags printType and printInitVal. Mode 3
/// is characterized by flag printImplementation set to true.
/// @arg Print of declarations by PrintHeaderVisitor. Declarations are printed
/// with their type but without initial value.
/// @arg Print of declarations by PrintImplementationVisitor. Declarations are
///	printed with initial value but without their type.
/// @arg Print of implementation, which concerns Statetables, Functions and
///	Procedures.
///
class PrintSystemCVisitor : public hif::GuideVisitor
{
public:
    using ObjectList       = std::list<hif::Object *>;
    using ConstTemplateMap = std::map<hif::Object *, ObjectList>;

    enum ConstTemplateContext {
        CONST_TEMPL_CTOR_DECL,
        CONST_TEMPL_CTOR_IMPL,
        CONST_TEMPL_CTOR_INIT_LIST,
        CONST_TEMPL_CTOR_CALL,
        CONST_TEMPL_DECL
    };

    /// @brief Constructor.
    PrintSystemCVisitor(
        hif::backends::IndentedStream *stream,
        PrintSystemCVisitorOptions &opt,
        PrintSystemCVisitor::ConstTemplateMap &ctmList,
        std::string baseName,
        std::string extension);
    PrintSystemCVisitor(ConstTemplateMap &ctmList);

    /// Destructor.
    ~PrintSystemCVisitor() override;

    enum OperatorPecedenceEnum {
        prec_min = 0,
        prec_concat,
        prec_assign,
        prec_when,
        prec_or,
        prec_and,
        prec_bor,
        prec_bxor,
        prec_band,
        prec_eq_neq,
        prec_gt_ge,
        prec_lt_le,
        prec_shifts,
        prec_plus_minus,
        prec_mult_div_mod,
        prec_ref,
        prec_deref,
        prec_cast,
        prec_not_bnot,
        prec_unary_plus_minus,
        prec_access,
        prec_member,
        prec_call,
        prec_scope,
        prec_max
    };

    /// @name Accessories.
    /// @{

    auto getCurrentDesignUnit() -> hif::DesignUnit *;
    void setCurrentDesignUnit(hif::DesignUnit *du);

    auto getCurrentLibraryDef() -> hif::LibraryDef *;
    void setCurrentLibraryDef(hif::LibraryDef *libDef);

    auto getDesignUnitScope() -> std::list<hif::DesignUnit *> &;
    void setDesignUnitScope(std::list<hif::DesignUnit *> &DUScope);

    auto getLibraryDefScope() -> std::list<hif::LibraryDef *> &;
    void setLibraryDefScope(std::list<hif::LibraryDef *> &LDScope);

    /// @}

    static void clearConstTemplateMap(ConstTemplateMap &list);

    /// @name Refinement methods.
    /// @{

    auto visitAggregate(hif::Aggregate &o) -> int override;
    auto visitAggregateAlt(hif::AggregateAlt &o) -> int override;
    auto visitAlias(hif::Alias &o) -> int override;
    auto visitArray(hif::Array &o) -> int override;
    auto visitAssign(hif::Assign &o) -> int override;
    auto visitBit(hif::Bit &o) -> int override;
    auto visitBitValue(hif::BitValue &o) -> int override;
    auto visitBitvector(hif::Bitvector &o) -> int override;
    auto visitBitvectorValue(hif::BitvectorValue &o) -> int override;
    auto visitBool(hif::Bool &o) -> int override;
    auto visitBoolValue(hif::BoolValue &o) -> int override;
    auto visitBreak(hif::Break &o) -> int override;
    auto visitCast(hif::Cast &o) -> int override;
    auto visitChar(hif::Char &o) -> int override;
    auto visitCharValue(hif::CharValue &o) -> int override;
    auto visitConst(hif::Const &o) -> int override;
    auto visitContents(hif::Contents &o) -> int override;
    auto visitContinue(hif::Continue &o) -> int override;
    auto visitDesignUnit(hif::DesignUnit &o) -> int override;
    auto visitEntity(hif::Entity &o) -> int override;
    auto visitEnum(hif::Enum &o) -> int override;
    auto visitEvent(hif::Event &o) -> int override;
    auto visitExpression(hif::Expression &o) -> int override;
    auto visitField(hif::Field &o) -> int override;
    auto visitFieldReference(hif::FieldReference &o) -> int override;
    auto visitFile(hif::File &o) -> int override;
    auto visitFor(hif::For &o) -> int override;
    auto visitFunction(hif::Function &o) -> int override;
    auto visitFunctionCall(hif::FunctionCall &o) -> int override;
    auto visitForGenerate(hif::ForGenerate &o) -> int override;
    auto visitGlobalAction(hif::GlobalAction &o) -> int override;
    auto visitIdentifier(hif::Identifier &o) -> int override;
    auto visitIf(hif::If &o) -> int override;
    auto visitIfAlt(hif::IfAlt &o) -> int override;
    auto visitIfGenerate(hif::IfGenerate &o) -> int override;
    auto visitInstance(hif::Instance &o) -> int override;
    auto visitInt(hif::Int &o) -> int override;
    auto visitIntValue(hif::IntValue &o) -> int override;
    auto visitLibrary(hif::Library &o) -> int override;
    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitMember(hif::Member &o) -> int override;
    auto visitTransition(hif::Transition &o) -> int override;
    auto visitNull(hif::Null &o) -> int override;
    auto visitParameter(hif::Parameter &o) -> int override;
    auto visitParameterAssign(hif::ParameterAssign &o) -> int override;
    auto visitPointer(hif::Pointer &o) -> int override;
    auto visitPort(hif::Port &o) -> int override;
    auto visitProcedure(hif::Procedure &o) -> int override;
    auto visitProcedureCall(hif::ProcedureCall &o) -> int override;
    auto visitReal(hif::Real &o) -> int override;
    auto visitRealValue(hif::RealValue &o) -> int override;
    auto visitRecord(hif::Record &o) -> int override;
    auto visitRecordValue(hif::RecordValue &o) -> int override;
    auto visitRecordValueAlt(hif::RecordValueAlt &o) -> int override;
    auto visitReference(hif::Reference &o) -> int override;
    auto visitReturn(hif::Return &o) -> int override;
    auto visitSignal(hif::Signal &o) -> int override;
    auto visitSigned(hif::Signed &o) -> int override;
    auto visitSlice(hif::Slice &o) -> int override;
    auto visitState(hif::State &o) -> int override;
    auto visitStateTable(hif::StateTable &o) -> int override;
    auto visitString(hif::String &o) -> int override;
    auto visitSystem(hif::System &o) -> int override;
    auto visitSwitch(hif::Switch &o) -> int override;
    auto visitSwitchAlt(hif::SwitchAlt &o) -> int override;
    auto visitStringValue(hif::StringValue &o) -> int override;
    auto visitTime(hif::Time &o) -> int override;
    auto visitTimeValue(hif::TimeValue &o) -> int override;
    auto visitTypeDef(hif::TypeDef &o) -> int override;
    auto visitTypeReference(hif::TypeReference &o) -> int override;
    auto visitTypeTP(hif::TypeTP &o) -> int override;
    auto visitTypeTPAssign(hif::TypeTPAssign &o) -> int override;
    auto visitUnsigned(hif::Unsigned &o) -> int override;
    auto visitValueStatement(hif::ValueStatement &o) -> int override;
    auto visitValueTP(hif::ValueTP &o) -> int override;
    auto visitValueTPAssign(hif::ValueTPAssign &o) -> int override;
    auto visitVariable(hif::Variable &o) -> int override;
    auto visitView(hif::View &o) -> int override;
    auto visitViewReference(hif::ViewReference &o) -> int override;
    auto visitWait(hif::Wait &o) -> int override;
    auto visitWhen(hif::When &o) -> int override;
    auto visitWhenAlt(hif::WhenAlt &o) -> int override;
    auto visitWhile(hif::While &o) -> int override;
    auto visitWith(hif::With &o) -> int override; // includes VisitWithAlt.
    auto visitWithAlt(hif::WithAlt &o) -> int override;

    /// @}

protected:
    /// @brief Support struct used to backup current visit mode.
    struct BackupOpt {
        PrintSystemCVisitorOptions _opt;
        std::list<hif::DesignUnit *> _design_unit_scope;
        std::list<hif::LibraryDef *> _library_def_scope;

        BackupOpt();
        ~BackupOpt();

        BackupOpt(const BackupOpt &);
        auto operator=(const BackupOpt &) -> BackupOpt &;
    };

private:
    PrintSystemCVisitor(const PrintSystemCVisitor &)               = delete;
    auto operator=(PrintSystemCVisitor &) -> PrintSystemCVisitor & = delete;

    /// @brief Backup the current visit mode.
    auto _backupVisitMode() -> BackupOpt;

    /// @brief Restore the previous visit mode.
    void _restoreVisitMode(BackupOpt &backupOtions);

    /// @name Check functions.
    /// @{

    /// @brief SystemC provides this feature on logic bits for sc_signal and sc_port
    /// sc_signal_resolved has type sc_logic, sc_signal_rv has type sc_lv
    /// the same is true for all kind of port (sc_in, sc_out, sc_inout)
    ///
    /// We assume that a port/signal has at most type array packed (not nested array)
    /// Signal/port visit will take care to understand if the type is sc_logic or sc_lv
    auto _isSystemCResolved(hif::Type *type) -> bool;

    /// @brief Function that check if add ";" ad end of statement. At the moment
    /// it checks only functions.
    auto _isStatement(hif::Object *obj) -> bool;

    /// @brief Check if the component is related to TLM, since they need a different
    /// management from other components.
    static auto _isTLMComponent(hif::DataDeclaration *obj) -> bool;

    /// @brief Check if a Function actually represents a C++ class constructor,
    /// or if a FunctionCall represents the call to a C++ class constructor.
    static auto _isCppConstructor(hif::Object *obj) -> bool;

    /// @brief Check if the function is actually the C++ class destructor.
    /// By convention, it is named __hif_destructor
    static auto _isCppDestructor(hif::Object *obj) -> bool;

    /// @brief Check if the function is native of HIF.
    static auto _isNativeHifFunction(hif::Object *obj) -> bool;

    /// @brief Check if the procedure is native of HIF.
    static auto _isNativeHifProcedure(hif::Object *obj) -> bool;

    /// @brief Return true if the function call need the additional qualifier
    /// ".template". It is required in this situation:
    /// @code
    /// { // templated scope
    ///     class.foo<i>()
    /// }
    /// @endcode
    ///
    /// conditions:
    /// 1) Both "class" and "foo" (member of "class") are templated.
    /// 2) "i" assignment must be explicit.
    /// 3) At least a parent scope is templated.
    ///
    template <typename T> auto _needTemplateAsQualifier(T *o) -> bool;

    /// @brief Check if the LibraryDef does not contain anything else than
    /// DesignUnits or other LibraryDefs, that are printed into independent files.
    static auto _containsOnlyIndependentComponents(hif::LibraryDef &o) -> bool;

    /// @brief Return true if given value is template.
    auto _isTemplateInstance(hif::Value *o) -> bool;

    /// @}

    /// @name Main printing functions.
    /// @{

    /// @brief Open a namespace basing on current LibraryDef name.
    void _openLibraryDefNamespace(hif::LibraryDef *ld, const std::string &libraryName = std::string());

    /// @brief Close a namespace basing on current LibraryDef name.
    void _closeLibraryDefNamespace(hif::LibraryDef *ld, const std::string &libraryName = std::string());

    /// @brief Prints library components (Views and and LibraryDefs) inclusion.
    void
    _printLibraryComponentInclusion(hif::LibraryDef *ld, const std::string &guardName, const std::string &includeName);

    /// @brief Print module constructor and destructor.
    void _printModuleCtorDtor(hif::View *duView);

    /// @}

    /// @name Print DesignUnit contents.
    /// @{

    /// @brief Access point to print DesignUnit contents,
    void _printModuleContents(hif::View *view);

    /// @brief Header print. Distinguish between public ones (i.e., Entity,
    /// Variables, TypeDef, Function) and private ones (i.e., StateTable).
    void _printModuleContents_H(hif::View *view);

    /// @brief Implementation print.
    void _printModuleContents_I(hif::View *view);

    /// @}

    /// @name Header (and implementation header) ones.
    /// @{

    /// @brief Prints the common string "<language> code generated by hif2sc".
    void _printCommonHeader(const std::string &filename);

    /// @brief Open pre-processor guard.
    void _printHeaderGuardBegin(const std::string &guardName);

    /// @brief Close pre-processor guard.
    void _printHeaderGuardEnd(hif::Object &o, const std::string &suggestedName = std::string());

    auto _calculateInclude(hif::Object *where, hif::Library *lib) -> std::string;

    /// @brief Print the inclusion of Library and other components (already managed
    /// in PostRefine_final Visitor).
    void _printIncludes(hif::BList<hif::Library> &list, hif::DesignUnit *du);

    /// @brief Entry point for DesignUnit.
    void _printModuleDeclaration(hif::DesignUnit &o);

    /// @brief Print module definition.
    void _printModuleBegin(hif::View *duView);

    /// @brief Print module closure.
    void _printModuleEnd();

    /// @brief Print inheritance from other DesignUnits.
    void _printModuleInheritance(hif::View *view);

    /// @brief Print module copy constructor and assign operator.
    void _printModuleCopyCtorAssignOp();

    /// @brief Entry point for LibraryDef.
    void _printLibraryDeclaration(hif::LibraryDef &o);

    /// @brief Entry point for System.
    void _printSystemDeclaration(hif::System &o);

    /// @brief Print SubProgram with kind MACRO.
    void _printMacro(hif::BList<hif::Declaration> &list);

    /// @brief Different management for printing of SystemC-AMS ports.
    /// @return True if the port is actually a SystemC-AMS one, False otherwise.
    auto _isAMSPort(hif::Port *o) -> bool;

    /// @name Functions to print LibraryDef or System declarations.
    /// @param declarations The list to print.
    /// @{

    /// @brief Access point to both header and implementation print.
    /// @param declarations The declarations
    /// @param startingObj Indicates the starting object.
    /// @param insideNamespace Indicates whether we are inside namespace parenthesis
    ///     (used for LibraryDef header only).
    void _printDeclarations(
        hif::BList<hif::Declaration> &declarations,
        hif::Object *startingObj,
        bool insideNamespace = false);

    /// @brief Header print.
    /// @param startingObj Indicates the starting object.
    /// @param insideNamespace Indicates whether we are inside namespace parenthesis.
    void
    _printDeclarations_H(hif::BList<hif::Declaration> &declarations, hif::Object *startingObj, bool insideNamespace);

    /// @brief Implementation print.
    /// @param startingObj Indicates the starting object.
    void _printDeclarations_I(hif::BList<hif::Declaration> &declarations, hif::Object *startingObj);

    /// @}

    /// @}

    /// @name Implementation functions.
    /// @{

    /// @brief Print the inclusions necessary to this implementation.
    void _printImplementationBegin(hif::Object *obj);

    /// @brief Entry point for DesignUnit.
    void _printModuleImplementation(hif::DesignUnit &o);

    /// @brief Delete of Instance Pointer inside module destructor.
    void _printDestructorInstanceDelete(hif::View *duView);

    /// @brief Check if constructor has need of an initialization list.
    auto _needInitializationList(hif::View *view) -> bool;

    /// @brief Print initialization list of constructor.
    auto _printInitializationList(hif::View *view) -> int;

    /// @brief Print initialization of variables that cannot be inserted in the
    /// module initialization list, or ones that need extra operations.
    auto _printOtherInitializations(hif::View *view) -> int;

    /// @brief Manage print of constants.
    /// @param declarations is the list of declarations to print.
    /// @param scope indicates the object containing the declarations
    /// of the constants (DesignUnit, LibraryDef, System).
    /// @param onlyDefines is the flag set by the caller, distinguishing if
    /// defines or "normal" constants have to be printed in that point
    void _printConstants(hif::BList<hif::Declaration> &declarations, hif::Object *scope, bool onlyDefines);

    /// @brief Entry point for LibraryDef.
    void _printLibraryImplementation(hif::LibraryDef &o);

    /// @brief Entry point for System.
    void _printSystemImplementation(hif::System &o);

    /// @brief Print initialization for output ports.
    void _printInitialize(hif::Port &o, hif::Type *portType, std::list<std::string> &indexes, bool isAMS);

    /// @brief Same of _printInitialize(), on port bindings.
    void
    _printPortBinding(const std::string &instName, hif::PortAssign *o, hif::Type *t, std::list<std::string> &indexes);

    /// @}

    /// @}

    /// @name Functions related to initialization.
    /// @{

    /// @brief Check the type of initialization that must be printed in constructor.
    /// Individual init. means that the class member has type Array or some other
    /// type that requires a peculiar (e.g., element by element) init. inside constructor
    /// body. Other peculiar cases are represented by TLM variables. Initialization
    /// is normally printed exploiting constructor initialization list.
    void _manageInitialization(hif::DataDeclaration *ddo);

    /// @brief Checks if the variable type is composed (i.e., Array or Record),
    /// thus individual initialization of each element is needed.
    /// 1. If the type is Array of Array, or a TypeReference of type Array.
    /// @param ddo The DataDeclaration to be analyzed.
    /// @return the Type to be considered for print.
    auto _needIndividualInit(hif::DataDeclaration *ddo) -> hif::Type *;

    /// @brief Perform individual initialization for declarations that need it.
    /// @param ddo The DataDeclaration involved.
    /// @param baseT The base type of ddo.
    void _printIndividualInit(hif::DataDeclaration *ddo, hif::Array *baseT);

    /// @brief Wrapper function for all cases of "normal" initialization.
    /// @return <tt>true</tt> if something is actually printed.
    auto _printNormalInit(hif::DataDeclaration *o) -> bool;

    /// @brief Prints the type of a signal.
    void _printSignalTypeAndName(hif::Signal *o);

    /// @brief Particular cases of initialization: TLM components.
    /// @param o The variable to analyze.
    /// @return True if this is the case, False otherwise.
    auto _printTLMInit(hif::DataDeclaration *o) -> bool;

    /// @brief Manage the particular double-initialization of Signal and Port,
    /// considering also variation for AMS.
    auto _initListInitializationSignalPort(hif::DataDeclaration *dd) -> bool;

    /// TODO
    auto _isFullySpecifiedArrayConst(hif::Const *c) -> bool;

    /// @brief Print fake class-fashion methods inside the Record declaration directly:
    /// 1- A constructor without parameters setting default values for all fields.
    /// 2- A constructor with parameters concerning all fields.
    /// 3- A destructor.
    /// Note: must be done at print-time since HIF do not accept other objects
    /// than Field inside Record.
    /// @param obj The Record to manage.
    void _printRecordClasslikeMethods(hif::Record *obj);

    /// @brief Print record operators ==, << and sc_trace
    void _printRecordSignalMethods(hif::Record *obj);

    /// @brief Print aggregate other in case of record value and using of cpp 98.
    auto _printAggregateRecordValue(hif::RecordValue *rv, hif::DataDeclaration *ddo) -> std::string;

    //	/// In case of Assign from Aggregate to XX introduce a new variable, move the
    //	/// assign to init.value, and assign the new variable to the previous one
    //	/// TODO move to PreRefines when interleaving of decls and stmts will be supported
    //	bool _manageAssignOfAggregate(Assign * o);

    /// @}

    /// @name Functions related to the visit of composite types
    /// @{

    /// @brief Composite types require a double visit of their type to achieve a correct
    /// print. This function is a wrapper that identifies the composite and calls
    /// the correspondent function.
    void _visitType(hif::Type *type, bool different_management);

    /// @brief Function used to visit an array, if it is candidate to be an array of array
    /// or an array of objects. This is intended as a safe way to use the
    /// flag printSquareSpan, which may influence further behavior. Thus, this function
    /// performs the visit and printSquareSpan is reset after that.
    void _visitTypeArray(hif::Array *type, bool use_printSquareSpan);

    /// @brief Functions used to visit a record, to print correctly its type (normally,
    /// "struct") on the first call and its field on the second call.
    void _visitTypeRecord(hif::Record *type, bool print_fields);

    /// @brief Functions used to visit a int, to print the bit field.
    void _visitTypeBitField(hif::Int *type, bool print_bitField);

    /// @}

    /// @brief Check whether print the keyword "typename" is necessary (i.e., if
    /// the list of TP assigns contains an identifier which is a template parameter in its turn).
    auto _isNeededTypename(hif::ReferencedType *refType) -> bool;

    /// @name Functions related to native HIF constructs.
    /// @{

    /// @brief Manage native FunctionCall to constructor.
    void _printCppConstructor(hif::FunctionCall &o);
    /// @brief Manage native ProcedureCall to destructor.
    void _printCppDestructor(hif::ProcedureCall &o);

    /// @brief Manage native FunctionCalls.
    void _printNativeFunctionCall(hif::FunctionCall &o);
    auto _printNativeFunctionCall_new(hif::FunctionCall &o) -> bool;
    auto _printNativeFunctionCall_malloc(hif::FunctionCall &o) -> bool;

    /// @brief Manage native ProcedureCalls.
    void _printNativeProcedureCall(hif::ProcedureCall &o);

    void _printInstanceBindingStatements(hif::Instance *inst, hif::Object *o, bool useArrow);

    template <typename T> auto _printCall(T &o) -> int;

    /// @}

    /// Print utility functions
    /// @{

    /// @brief Print of StateTable implementation.
    /// This function is a wrapper for the print of actual StateTable code
    /// that can be exploited by other visits (e.g., Function, Procedure.
    /// Note: header and trailer of StateTable are managed here.
    auto _printStateTable(hif::StateTable *st, hif::DesignUnit *du) -> int;

    /// @brief Print a for loop into the generated code.
    /// This is useful for operations like initialization of array, etc.
    void
    _printForLoopHeader(const std::string &indexName, hif::Type *indexType, hif::Range *range, hif::Object *treeObject);

    /// @brief Print sensitivity list of a Statetable.
    /// @param isPos is used to know if positive-related keyword must be printed.
    /// @param isNeg is used to know if negative-related keyword must be printed.
    void _printSensitivity(hif::BList<hif::Value> &sensitivity, bool isPos = false, bool isNeg = false);

    /// @brief Print sensitivity-list item.
    /// It distinguishes between names of single port/signals and arrays of
    /// port/signals. The appropriate keyword for ports or signals is also generated.
    /// @param isPos is used to know if positive-related keyword must be printed.
    /// @param isNeg is used to know if negative-related keyword must be printed.
    /// @param freshStart is used to know when the sensitivity list has to be
    /// splitted in more parts.
    void _printSensitivityItem(hif::Value *name, bool *freshStart, bool isPos = false, bool isNeg = false);

    /// @brief Print suffix related to posedge/negedge, basing on the type of
    /// declaration (differentiate print for signals and ports).
    void _printSensitivitySuffix(hif::Value *name, bool isPos, bool isNeg);

    /// @brief In case of arrays, print a loop (or a series of nested loops) to
    /// manage sensitivity on each element.
    void _printSensitivityLoop(
        hif::Value *name,
        hif::Type *nameType,
        bool isPos,
        bool isNeg,
        std::list<std::string> &indexes);

    /// @brief Print SubProgram declaration.
    /// @param altName allows to specify a different name to be printed if
    /// necessary.
    void _printSubProgramDeclaration(hif::SubProgram &o, const std::string &altName = std::string());

    /// @brief Print SubProgram implementation.
    /// @param altName allows to specify a different name to be printed if
    /// necessary.
    void _printSubProgramImplementation(hif::SubProgram &o, const std::string &altName = std::string());

    /// @brief Temporary replace a bound to print the size of given span
    /// @param span The span.
    void _printTypeSpanSize(hif::Range *span);

    /// @brief Options related to _printList method.
    struct PrintListOpt {
        /// @brief indicates if it is mandatory to put the list inside parenthesis.
        bool _mandatoryParen   = false;
        /// @brief indicates if it is mandatory to avoid parenthesis.
        bool _mandatoryNoParen = false;
        /// @brief indicates if the parenthesis must be angular (default: rounded).
        bool _angularParen     = false;
        /// @brief indicates if the parenthesis must be curly (default: rounded).
        bool _curlyParen       = false;
        /// @brief indicates if each element of the list must be on a new line.
        bool _breakLine        = false;
    };

    /// @brief Print a list of objects.
    /// @param list The list to be printed.
    /// @param opt The print options (see _printList_opt documentation).
    template <class T> void _printList(hif::BList<T> &list, PrintListOpt opt);

    /// @brief Print the list of comments associated to the current object.
    void _printComment(hif::Object *o);

    /// @brief Print eventual define associated to the current object.
    void _printDefineMacros(hif::Object *o);

    /// @brief Print the list of additional keywords associated to the object (if any)
    void _printAdditionalKeywords(hif::Declaration *o);

    /// @brief Print template parameters defined on each scope of the object
    /// that is being visited, basing on info collected during visit.
    void _printScopeTemplate();

    /// @brief Print scope of the object that is being visited, basing on info
    /// collected during visit.
    void _printScopeHierarchy();

    /// @brief Return the given string in capital letters.
    static auto _capitalize(const char *str) -> std::string;

    /// @brief Returns true if given expression type is native int 32 bits.
    auto _isInt32Type(hif::Expression *e) -> bool;

    /// @brief Returns true if printing of given IntValue may be ambiguous.
    auto _mayBeAmbiguous(hif::IntValue *o) -> bool;

    /// @brief Returns true if printing of given StringValue may be ambiguous.
    auto _mayBeAmbiguous(hif::StringValue *o) -> bool;

    /// @}

    /// @name Functions that manage print of template parameters.
    /// @{

    /// @brief Print template parameters complete of their type and initial value.
    // Example: < int ticks = 3, bool last_in_chain = true >
    auto _printFullTP(hif::BList<hif::Declaration> &temp_params) -> int;

    /// @brief Print template parameters with their type but without initial value.
    // Example: < int ticks, bool last_in_chain >
    auto _printTypedTP(hif::BList<hif::Declaration> &temp_params) -> bool;

    /// @brief Print template parameters without their type and initial value.
    // Example: < ticks, last_in_chain >
    auto _printUntypedTP(hif::BList<hif::Declaration> &temp_params) -> int;

    /// @brief Prints template parameters in different modes.
    auto
    _printTemplateParameters(hif::BList<hif::Declaration> &temp_params, bool typed = false, bool init = false) -> bool;

    void _printNotCompileTimeTemplates(hif::Object *o, ConstTemplateContext c);

    /// @}

    auto _needWrapParen(hif::Object *v) -> bool;
    auto _getOperatorPrecedence(hif::Object *v) -> OperatorPecedenceEnum;

    /// @brief Visit options
    PrintSystemCVisitorOptions _opt;

    /// @brief The reference semantics.
    hif::semantics::SystemCSemantics *_sem;

    /// @brief The map of constant template.
    PrintSystemCVisitor::ConstTemplateMap &_ctmList;

    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_outstream;

    /// @brief Keep trace of DesignUnit scope.
    std::list<hif::DesignUnit *> _design_unit_scope;

    /// @brief Keep trace of LibraryDef scope.
    std::list<hif::LibraryDef *> _library_def_scope;

    /// @brief The number of left angular brachets opened.
    long long int _left_angular;

    /// @brief The data types string used for library defs.
    const std::string _dataTypesString;

    /// @brief The base name of output file base name;
    const std::string _baseName;

    /// @brief The base name of output file extension;
    const std::string _current_file_extension;
};
