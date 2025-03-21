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
    typedef std::set<hif::View *> ViewSet;
    typedef std::map<hif::View *, ViewSet> ViewMap;

    PrintVerilogVisitor(hif::backends::IndentedStream *outstream, std::string baseName, std::string extension);

    virtual ~PrintVerilogVisitor();

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

    virtual int visitEnumValue(hif::EnumValue &o);

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

    virtual int visitPortAssign(hif::PortAssign &o);

    virtual int visitPort(hif::Port &o);

    virtual int visitProcedure(hif::Procedure &o);

    virtual int visitRange(hif::Range &o);

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

private:
    PrintVerilogVisitor(const PrintVerilogVisitor &);

    PrintVerilogVisitor &operator=(const PrintVerilogVisitor &);

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
    static int _createDirectory(const std::string &dirName);

    /// @brief Function to check if it is a support declaration.
    bool _isSupportDeclaration(hif::Declaration *d);

    /// @brief Function to check if starts with str.
    static bool _startsWith(const std::string &str, const std::string &target);

    /// @brief Function to check if ends with str.
    static bool _endsWith(const std::string &str, const std::string &target);

    /// @name Print a list of objects.
    /// @param list The list to be printed.
    /// @param separator The separator among elements of \p list.
    /// @param needNewLine If true, a new line is added at the end of each
    /// element.
    /// @{

    template <class T> void _printList(hif::BList<T> &list, const char separator, const bool needNewLine);

    template <class T> void _printList(hif::BList<T> &list, const std::string &separator, const bool needNewLine);

    void _printList(hif::BList<hif::Object> &list, const std::string &separator, const bool needNewLine);

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
    bool _printFileVariable(hif::Variable *o);

    /// @brief Special printing for assert statement.
    bool _printAssertStatement(hif::ProcedureCall *o);

    /// @brief Floating point comparison.
    static bool _approximatelyEqual(double a, double b, double epsilon);

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
