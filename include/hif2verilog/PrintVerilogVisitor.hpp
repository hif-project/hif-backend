/// @file PrintVerilogVisitor.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <hif/hif.hpp>

class PrintVerilogVisitor : public hif::GuideVisitor
{
public:
    using ViewSet = std::set<hif::View *>;
    using ViewMap = std::map<hif::View *, ViewSet>;

    PrintVerilogVisitor(hif::backends::IndentedStream *outstream, std::string baseName, std::string extension);

    ~PrintVerilogVisitor() override;

    auto visitAggregate(hif::Aggregate &o) -> int override ;

    auto visitAggregateAlt(hif::AggregateAlt &o) -> int override ;

    auto visitAlias(hif::Alias &o) -> int override ;

    auto visitArray(hif::Array &o) -> int override ;

    auto visitAssign(hif::Assign &o) -> int override ;

    auto visitBit(hif::Bit &o) -> int override ;

    auto visitBitValue(hif::BitValue &o) -> int override ;

    auto visitBitvector(hif::Bitvector &o) -> int override ;

    auto visitBitvectorValue(hif::BitvectorValue &o) -> int override ;

    auto visitBool(hif::Bool &o) -> int override ;

    auto visitBoolValue(hif::BoolValue &o) -> int override ;

    auto visitBreak(hif::Break &o) -> int override ;

    auto visitCast(hif::Cast &o) -> int override ;

    auto visitChar(hif::Char &o) -> int override ;

    auto visitCharValue(hif::CharValue &o) -> int override ;

    auto visitConst(hif::Const &o) -> int override ;

    auto visitContents(hif::Contents &o) -> int override ;

    auto visitContinue(hif::Continue &o) -> int override ;

    auto visitDesignUnit(hif::DesignUnit &o) -> int override ;

    auto visitEntity(hif::Entity &o) -> int override ;

    auto visitEnum(hif::Enum &o) -> int override ;

    auto visitEnumValue(hif::EnumValue &o) -> int override ;

    auto visitEvent(hif::Event &o) -> int override ;

    auto visitExpression(hif::Expression &o) -> int override ;

    auto visitField(hif::Field &o) -> int override ;

    auto visitFieldReference(hif::FieldReference &o) -> int override ;

    auto visitFile(hif::File &o) -> int override ;

    auto visitFor(hif::For &o) -> int override ;

    auto visitFunction(hif::Function &o) -> int override ;

    auto visitFunctionCall(hif::FunctionCall &o) -> int override ;

    auto visitForGenerate(hif::ForGenerate &o) -> int override ;

    auto visitGlobalAction(hif::GlobalAction &o) -> int override ;

    auto visitIdentifier(hif::Identifier &o) -> int override ;

    auto visitIf(hif::If &o) -> int override ;

    auto visitIfAlt(hif::IfAlt &o) -> int override ;

    auto visitIfGenerate(hif::IfGenerate &o) -> int override ;

    auto visitInstance(hif::Instance &o) -> int override ;

    auto visitInt(hif::Int &o) -> int override ;

    auto visitIntValue(hif::IntValue &o) -> int override ;

    auto visitLibrary(hif::Library &o) -> int override ;

    auto visitLibraryDef(hif::LibraryDef &o) -> int override ;

    auto visitMember(hif::Member &o) -> int override ;

    auto visitTransition(hif::Transition &o) -> int override ;

    auto visitNull(hif::Null &o) -> int override ;

    auto visitParameter(hif::Parameter &o) -> int override ;

    auto visitParameterAssign(hif::ParameterAssign &o) -> int override ;

    auto visitPointer(hif::Pointer &o) -> int override ;

    auto visitPortAssign(hif::PortAssign &o) -> int override ;

    auto visitPort(hif::Port &o) -> int override ;

    auto visitProcedure(hif::Procedure &o) -> int override ;

    auto visitRange(hif::Range &o) -> int override ;

    auto visitProcedureCall(hif::ProcedureCall &o) -> int override ;

    auto visitReal(hif::Real &o) -> int override ;

    auto visitRealValue(hif::RealValue &o) -> int override ;

    auto visitRecord(hif::Record &o) -> int override ;

    auto visitRecordValue(hif::RecordValue &o) -> int override ;

    auto visitRecordValueAlt(hif::RecordValueAlt &o) -> int override ;

    auto visitReference(hif::Reference &o) -> int override ;

    auto visitReturn(hif::Return &o) -> int override ;

    auto visitSignal(hif::Signal &o) -> int override ;

    auto visitSigned(hif::Signed &o) -> int override ;

    auto visitSlice(hif::Slice &o) -> int override ;

    auto visitState(hif::State &o) -> int override ;

    auto visitStateTable(hif::StateTable &o) -> int override ;

    auto visitString(hif::String &o) -> int override ;

    auto visitSystem(hif::System &o) -> int override ;

    auto visitSwitch(hif::Switch &o) -> int override ;

    auto visitSwitchAlt(hif::SwitchAlt &o) -> int override ;

    auto visitStringValue(hif::StringValue &o) -> int override ;

    auto visitTime(hif::Time &o) -> int override ;

    auto visitTimeValue(hif::TimeValue &o) -> int override ;

    auto visitTypeDef(hif::TypeDef &o) -> int override ;

    auto visitTypeReference(hif::TypeReference &o) -> int override ;

    auto visitTypeTP(hif::TypeTP &o) -> int override ;

    auto visitTypeTPAssign(hif::TypeTPAssign &o) -> int override ;

    auto visitUnsigned(hif::Unsigned &o) -> int override ;

    auto visitValueStatement(hif::ValueStatement &o) -> int override ;

    auto visitValueTP(hif::ValueTP &o) -> int override ;

    auto visitValueTPAssign(hif::ValueTPAssign &o) -> int override ;

    auto visitVariable(hif::Variable &o) -> int override ;

    auto visitView(hif::View &o) -> int override ;

    auto visitViewReference(hif::ViewReference &o) -> int override ;

    auto visitWait(hif::Wait &o) -> int override ;

    auto visitWhen(hif::When &o) -> int override ;

    auto visitWhenAlt(hif::WhenAlt &o) -> int override ;

    auto visitWhile(hif::While &o) -> int override ;

    auto visitWith(hif::With &o) -> int override ; // includes VisitWithAlt.
    auto visitWithAlt(hif::WithAlt &o) -> int override ;

private:
    PrintVerilogVisitor(const PrintVerilogVisitor &);

    auto operator=(const PrintVerilogVisitor &) -> PrintVerilogVisitor &;

    /// @brief The reference semantics.
    hif::semantics::ILanguageSemantics *_sem;

    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_outstream;

    /// @brief The output directory where the Verilog output files are created.
    std::string _outDir;

    /// @brief The design unit name of the current sub-tree.
    std::string _currentDesignUnitName;

    /// @brief The view name of the current sub-tree.
    std::string _currentViewName;

    /// @brief The templateParameterAssignList of the current sub-tree.
    hif::View *_currentView{};

    /// @brief Hif System of the current sub-tree.
    hif::System *_currentSystem{};

    /// @brief Hif Contents of the current sub-tree.
    hif::Contents *_currentContents{};

    /// @brief Hif Entity of the current sub-tree.
    hif::Entity *_currentEntity{};

    /// @brief Store the components already printed.
    ViewMap _printedComponents;

    /// @brief Output file name.
    const std::string _baseName;

    /// @brief Output file extension.
    const std::string _extension;

    /// @brief Variable to check if the range is composed by Real Value.
    bool _isRealRange{};

    /// @brief Variable to check if AMS is enabled.
    bool _ams_enabled{};

    /// @brief Variable to check if it is a print component.
    bool _isPrintComponents{};

    /// @brief Variable to check if it is a print condition.
    bool _isPrintWithCondition{};

    /// @brief Variable to check if it a library declaration.
    bool _isPrintingLibDefDecls{};

    /// @brief Variable to check if it is a sub-program body.
    bool _isSubProgramBody{};

    /// @brief Function to initialize the output stream.
    void _initializeOutstream(const std::string &fileName, const std::string &subdirectory);

    /// @brief Function to create the output directory.
    static auto _createDirectory(const std::string &dirName) -> int;

    /// @brief Function to check if it is a support declaration.
    static auto _isSupportDeclaration(hif::Declaration *d) -> bool;

    /// @brief Function to check if starts with str.
    static auto _startsWith(const std::string &str, const std::string &target) -> bool;

    /// @brief Function to check if ends with str.
    static auto _endsWith(const std::string &str, const std::string &target) -> bool;

    /// @name Print a list of objects.
    /// @param list The list to be printed.
    /// @param separator The separator among elements of \p list.
    /// @param needNewLine If true, a new line is added at the end of each
    /// element.
    /// @{

    template <class T> void _printList(hif::BList<T> &list, char separator, bool needNewLine);

    template <class T> void _printList(hif::BList<T> &list, const std::string &separator, bool needNewLine);

    void _printList(hif::BList<hif::Object> &list, const std::string &separator, bool needNewLine);

    /// @}

    /// @brief Function to check if the range of values is Real.
    void _setRealRange(hif::Range *o);

    /// @brief Function to print the instance value.
    void _printValueInstance(hif::Value *v);

    /// @brief Function to print the instance type.
    void _printTypeInstance(hif::ReferencedType *v);

    /// @brief Print the port direction corresponding string.
    void _printPortDirection(hif::PortDirection dir);

    /// @brief Print the libraries list of a DesignUnit.
    void _printLibraries(hif::BList<hif::Library> &libraries);

    /// @brief Special printing for file variables.
    auto _printFileVariable(hif::Variable *o) -> bool;

    /// @brief Special printing for assert statement.
    auto _printAssertStatement(hif::ProcedureCall *o) -> bool;

    /// @brief Floating point comparison.
    static auto _approximatelyEqual(double a, double b, double epsilon) -> bool;

    /// @brief Processes all declarations within the given contents.
    /// @param o The contents containing declarations.
    void processDeclarations(hif::Contents &o);

    /// @brief Processes non-AMS (Analog Mixed Signal) declarations.
    /// @param decl The declaration to process.
    void processNonAMSDeclaration(hif::Declaration *decl);

    /// @brief Prints a wire declaration for the given variable.
    /// @param v The variable to print as a wire.
    void printWire(hif::Variable *v);

    /// @brief Prints a register declaration for the given signal.
    /// @param s The signal to print as a register.
    void printReg(hif::Signal *s);

    /// @brief Prints the bit width for the given type.
    /// @param type The type to get the bit width from.
    void printBitwidth(hif::Type *type);

    /// @brief Prints the value for the given object if applicable.
    /// @param value The value object to print.
    void printValue(hif::Value *value);

    /// @brief Processes AMS (Analog Mixed Signal) specific declarations.
    /// @param decl The declaration to process.
    void processAMSDeclaration(hif::Declaration *decl);

    /// @brief Prints an alias declaration.
    /// @param a The alias to print.
    void printAlias(hif::Alias *a);

    /// @brief Prints an AMS-specific variable declaration.
    /// @param v The variable to print.
    void printAMSVariable(hif::Variable *v);

    /// @brief Prints a view reference for AMS variables.
    /// @param portType The view reference type.
    /// @param pName The name of the port.
    void printAMSViewReference(hif::ViewReference *portType, const std::string &pName);

    /// @brief Prints a type reference for AMS variables.
    /// @param portTypeRef The type reference.
    /// @param pName The name of the port.
    void printAMSTypeReference(hif::TypeReference *portTypeRef, const std::string &pName);

    /// @brief Processes state tables in the given contents.
    /// @param o The contents containing state tables.
    void processStateTables(hif::Contents &o);
};
