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
    PrintSystemCVisitorOptions &operator=(const PrintSystemCVisitorOptions &);
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
    typedef std::list<hif::Object *> ObjectList;
    typedef std::map<hif::Object *, ObjectList> ConstTemplateMap;

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
        const std::string &baseName,
        const std::string &extension);
    PrintSystemCVisitor(ConstTemplateMap &ctmList);

    /// Destructor.
    virtual ~PrintSystemCVisitor();

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

    hif::DesignUnit *getCurrentDesignUnit();
    void setCurrentDesignUnit(hif::DesignUnit *du);

    hif::LibraryDef *getCurrentLibraryDef();
    void setCurrentLibraryDef(hif::LibraryDef *libDef);

    std::list<hif::DesignUnit *> &getDesignUnitScope();
    void setDesignUnitScope(std::list<hif::DesignUnit *> &DUScope);

    std::list<hif::LibraryDef *> &getLibraryDefScope();
    void setLibraryDefScope(std::list<hif::LibraryDef *> &LDScope);

    /// @}

    static void clearConstTemplateMap(ConstTemplateMap &list);

    /// @name Refinement methods.
    /// @{

    virtual int visitAggregate(hif::Aggregate &o);
    virtual int visitAggregateAlt(hif::AggregateAlt &o);
    virtual int visitAlias(hif::Alias &o);
    virtual int visitArray(hif::Array &o);
    virtual int visitAssign(hif::Assign &o);
    virtual int visitBit(hif::Bit &o);
    virtual int visitBitValue(hif::BitValue &o);
    virtual int visitBitvector(hif::Bitvector &o);
    virtual int visitBitvectorValue(hif::BitvectorValue &o);
    virtual int visitBool(hif::Bool &o);
    virtual int visitBoolValue(hif::BoolValue &o);
    virtual int visitBreak(hif::Break &o);
    virtual int visitCast(hif::Cast &o);
    virtual int visitChar(hif::Char &o);
    virtual int visitCharValue(hif::CharValue &o);
    virtual int visitConst(hif::Const &o);
    virtual int visitContents(hif::Contents &o);
    virtual int visitContinue(hif::Continue &o);
    virtual int visitDesignUnit(hif::DesignUnit &o);
    virtual int visitEntity(hif::Entity &o);
    virtual int visitEnum(hif::Enum &o);
    virtual int visitEvent(hif::Event &o);
    virtual int visitExpression(hif::Expression &o);
    virtual int visitField(hif::Field &o);
    virtual int visitFieldReference(hif::FieldReference &o);
    virtual int visitFile(hif::File &o);
    virtual int visitFor(hif::For &o);
    virtual int visitFunction(hif::Function &o);
    virtual int visitFunctionCall(hif::FunctionCall &o);
    virtual int visitForGenerate(hif::ForGenerate &o);
    virtual int visitGlobalAction(hif::GlobalAction &o);
    virtual int visitIdentifier(hif::Identifier &o);
    virtual int visitIf(hif::If &o);
    virtual int visitIfAlt(hif::IfAlt &o);
    virtual int visitIfGenerate(hif::IfGenerate &o);
    virtual int visitInstance(hif::Instance &o);
    virtual int visitInt(hif::Int &o);
    virtual int visitIntValue(hif::IntValue &o);
    virtual int visitLibrary(hif::Library &o);
    virtual int visitLibraryDef(hif::LibraryDef &o);
    virtual int visitMember(hif::Member &o);
    virtual int visitTransition(hif::Transition &o);
    virtual int visitNull(hif::Null &o);
    virtual int visitParameter(hif::Parameter &o);
    virtual int visitParameterAssign(hif::ParameterAssign &o);
    virtual int visitPointer(hif::Pointer &o);
    virtual int visitPort(hif::Port &o);
    virtual int visitProcedure(hif::Procedure &o);
    virtual int visitProcedureCall(hif::ProcedureCall &o);
    virtual int visitReal(hif::Real &o);
    virtual int visitRealValue(hif::RealValue &o);
    virtual int visitRecord(hif::Record &o);
    virtual int visitRecordValue(hif::RecordValue &o);
    virtual int visitRecordValueAlt(hif::RecordValueAlt &o);
    virtual int visitReference(hif::Reference &o);
    virtual int visitReturn(hif::Return &o);
    virtual int visitSignal(hif::Signal &o);
    virtual int visitSigned(hif::Signed &o);
    virtual int visitSlice(hif::Slice &o);
    virtual int visitState(hif::State &o);
    virtual int visitStateTable(hif::StateTable &o);
    virtual int visitString(hif::String &o);
    virtual int visitSystem(hif::System &o);
    virtual int visitSwitch(hif::Switch &o);
    virtual int visitSwitchAlt(hif::SwitchAlt &o);
    virtual int visitStringValue(hif::StringValue &o);
    virtual int visitTime(hif::Time &o);
    virtual int visitTimeValue(hif::TimeValue &o);
    virtual int visitTypeDef(hif::TypeDef &o);
    virtual int visitTypeReference(hif::TypeReference &o);
    virtual int visitTypeTP(hif::TypeTP &o);
    virtual int visitTypeTPAssign(hif::TypeTPAssign &o);
    virtual int visitUnsigned(hif::Unsigned &o);
    virtual int visitValueStatement(hif::ValueStatement &o);
    virtual int visitValueTP(hif::ValueTP &o);
    virtual int visitValueTPAssign(hif::ValueTPAssign &o);
    virtual int visitVariable(hif::Variable &o);
    virtual int visitView(hif::View &o);
    virtual int visitViewReference(hif::ViewReference &o);
    virtual int visitWait(hif::Wait &o);
    virtual int visitWhen(hif::When &o);
    virtual int visitWhenAlt(hif::WhenAlt &o);
    virtual int visitWhile(hif::While &o);
    virtual int visitWith(hif::With &o); // includes VisitWithAlt.
    virtual int visitWithAlt(hif::WithAlt &o);

    /// @}

protected:
    /// @brief Support struct used to backup current visit mode.
    struct BackupOpt {
        PrintSystemCVisitorOptions _opt;
        std::list<hif::DesignUnit *> _DesignUnitScope;
        std::list<hif::LibraryDef *> _LibraryDefScope;

        BackupOpt();
        ~BackupOpt();

        BackupOpt(const BackupOpt &);
        BackupOpt &operator=(const BackupOpt &);
    };

private:
    PrintSystemCVisitor(const PrintSystemCVisitor &);
    const PrintSystemCVisitor &operator=(PrintSystemCVisitor &);

    /// @brief Backup the current visit mode.
    BackupOpt _backupVisitMode();

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
    bool _isSystemCResolved(hif::Type *type);

    /// @brief Function that check if add ";" ad end of statement. At the moment
    /// it checks only functions.
    bool _isStatement(hif::Object *obj);

    /// @brief Check if the component is related to TLM, since they need a different
    /// management from other components.
    bool _isTLMComponent(hif::DataDeclaration *obj);

    /// @brief Check if a Function actually represents a C++ class constructor,
    /// or if a FunctionCall represents the call to a C++ class constructor.
    bool _isCppConstructor(hif::Object *obj);

    /// @brief Check if the function is actually the C++ class destructor.
    /// By convention, it is named __hif_destructor
    bool _isCppDestructor(hif::Object *obj);

    /// @brief Check if the function is native of HIF.
    bool _isNativeHifFunction(hif::Object *obj);

    /// @brief Check if the procedure is native of HIF.
    bool _isNativeHifProcedure(hif::Object *obj);

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
    template <typename T> bool _needTemplateAsQualifier(T *o);

    /// @brief Check if the LibraryDef does not contain anything else than
    /// DesignUnits or other LibraryDefs, that are printed into independent files.
    bool _containsOnlyIndependentComponents(hif::LibraryDef &o);

    /// @brief Return true if given value is template.
    bool _isTemplateInstance(hif::Value *o);

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
    void _printCommonHeader(const std::string &filename, hif::LanguageID language);

    /// @brief Open pre-processor guard.
    void _printHeaderGuardBegin(const std::string &guardName);

    /// @brief Close pre-processor guard.
    void _printHeaderGuardEnd(hif::Object &o, const std::string &suggestedName = "");

    std::string _calculateInclude(hif::Object *where, hif::Library *lib);

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
    bool _isAMSPort(hif::Port *o);

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
    bool _needInitializationList(hif::View *view);

    /// @brief Print initialization list of constructor.
    int _printInitializationList(hif::View *view);

    /// @brief Print initialization of variables that cannot be inserted in the
    /// module initialization list, or ones that need extra operations.
    int _printOtherInitializations(hif::View *view);

    /// @brief Manage print of constants.
    /// @param declarations is the list of declarations to print.
    /// @param scope indicates the object containing the declarations
    /// of the constants (DesignUnit, LibraryDef, System).
    /// @param onlyDefines is the flag set by the caller, distinguishing if
    /// defines or "normal" constants have to be printed in that point
    void _printConstants(hif::BList<hif::Declaration> &declarations, hif::Object *scope, const bool onlyDefines);

    /// @brief Entry point for LibraryDef.
    void _printLibraryImplementation(hif::LibraryDef &o);

    /// @brief Entry point for System.
    void _printSystemImplementation(hif::System &o);

    /// @brief Print initialization for output ports.
    void _printInitialize(hif::Port &o, hif::Type *portType, std::list<std::string> &indexes, const bool isAMS);

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
    hif::Type *_needIndividualInit(hif::DataDeclaration *ddo);

    /// @brief Perform individual initialization for declarations that need it.
    /// @param ddo The DataDeclaration involved.
    /// @param baseT The base type of ddo.
    void _printIndividualInit(hif::DataDeclaration *ddo, hif::Array *baseT);

    /// @brief Wrapper function for all cases of "normal" initialization.
    /// @return <tt>true</tt> if something is actually printed.
    bool _printNormalInit(hif::DataDeclaration *o);

    /// @brief Prints the type of a signal.
    void _printSignalTypeAndName(hif::Signal *o);

    /// @brief Particular cases of initialization: TLM components.
    /// @param o The variable to analyze.
    /// @return True if this is the case, False otherwise.
    bool _printTLMInit(hif::DataDeclaration *o);

    /// @brief Manage the particular double-initialization of Signal and Port,
    /// considering also variation for AMS.
    bool _initListInitializationSignalPort(hif::DataDeclaration *dd);

    /// TODO
    bool _isFullySpecifiedArrayConst(hif::Const *c);

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
    std::string _printAggregateRecordValue(hif::RecordValue *rv, hif::DataDeclaration *ddo);

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
    bool _isNeededTypename(hif::ReferencedType *refType);

    /// @name Functions related to native HIF constructs.
    /// @{

    /// @brief Manage native FunctionCall to constructor.
    void _printCppConstructor(hif::FunctionCall &o);
    /// @brief Manage native ProcedureCall to destructor.
    void _printCppDestructor(hif::ProcedureCall &o);

    /// @brief Manage native FunctionCalls.
    void _printNativeFunctionCall(hif::FunctionCall &o);
    bool _printNativeFunctionCall_new(hif::FunctionCall &o);
    bool _printNativeFunctionCall_malloc(hif::FunctionCall &o);

    /// @brief Manage native ProcedureCalls.
    void _printNativeProcedureCall(hif::ProcedureCall &o);

    void _printInstanceBindingStatements(hif::Instance *inst, hif::Object *o, const bool useArrow);

    template <typename T> int _printCall(T &o);

    /// @}

    /// Print utility functions
    /// @{

    /// @brief Print of StateTable implementation.
    /// This function is a wrapper for the print of actual StateTable code
    /// that can be exploited by other visits (e.g., Function, Procedure.
    /// Note: header and trailer of StateTable are managed here.
    int _printStateTable(hif::StateTable *st, hif::DesignUnit *du);

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
        bool _mandatoryParen;
        /// @brief indicates if it is mandatory to avoid parenthesis.
        bool _mandatoryNoParen;
        /// @brief indicates if the parenthesis must be angular (default: rounded).
        bool _angularParen;
        /// @brief indicates if the parenthesis must be curly (default: rounded).
        bool _curlyParen;
        /// @brief indicates if each element of the list must be on a new line.
        bool _breakLine;

        PrintListOpt();
        PrintListOpt(
            const bool mandatoryParen,
            const bool mandatoryNoParen,
            const bool angularParen,
            const bool curlyParen,
            const bool breakLine);
        ~PrintListOpt();
        PrintListOpt(const PrintListOpt &other);
        PrintListOpt &operator=(const PrintListOpt &other);
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
    std::string _capitalize(const char *str);

    /// @brief Returns true if given expression type is native int 32 bits.
    bool _isInt32Type(hif::Expression *e);

    /// @brief Returns true if printing of given IntValue may be ambiguous.
    bool _mayBeAmbiguous(hif::IntValue *o);

    /// @brief Returns true if printing of given StringValue may be ambiguous.
    bool _mayBeAmbiguous(hif::StringValue *o);

    /// @}

    /// @name Functions that manage print of template parameters.
    /// @{

    /// @brief Print template parameters complete of their type and initial value.
    // Example: < int ticks = 3, bool last_in_chain = true >
    int _printFullTP(hif::BList<hif::Declaration> &temp_params);

    /// @brief Print template parameters with their type but without initial value.
    // Example: < int ticks, bool last_in_chain >
    bool _printTypedTP(hif::BList<hif::Declaration> &temp_params);

    /// @brief Print template parameters without their type and initial value.
    // Example: < ticks, last_in_chain >
    int _printUntypedTP(hif::BList<hif::Declaration> &temp_params);

    /// @brief Prints template parameters in different modes.
    bool _printTemplateParameters(hif::BList<hif::Declaration> &temp_params, bool typed = false, bool init = false);

    void _printNotCompileTimeTemplates(hif::Object *o, ConstTemplateContext c);

    /// @}

    bool _needWrapParen(hif::Object *v);
    OperatorPecedenceEnum _getOperatorPrecedence(hif::Object *v);

    /// @brief Visit options
    PrintSystemCVisitorOptions _opt;

    /// @brief The reference semantics.
    hif::semantics::SystemCSemantics *_sem;

    /// @brief The map of constant template.
    PrintSystemCVisitor::ConstTemplateMap &_ctmList;

    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_outstream;

    /// @brief Keep trace of DesignUnit scope.
    std::list<hif::DesignUnit *> _DesignUnitScope;

    /// @brief Keep trace of LibraryDef scope.
    std::list<hif::LibraryDef *> _LibraryDefScope;

    /// @brief The number of left angular brachets opened.
    long long int _leftAngular;

    /// @brief The data types string used for library defs.
    const std::string _dataTypesString;

    /// @brief The base name of output file base name;
    const std::string _baseName;

    /// @brief The base name of output file extension;
    const std::string _currentFileExtension;
};
