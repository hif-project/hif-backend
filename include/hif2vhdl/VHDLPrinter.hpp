/// @file VHDLPrinter.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <fstream>

#include <hif/hif.hpp>

class VHDLPrinter : public hif::GuideVisitor
{
public:
    typedef std::set<hif::View *> ViewSet;
    typedef std::map<hif::View *, ViewSet> ViewMap;

    VHDLPrinter(const std::string &outDir);
    virtual ~VHDLPrinter();

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
    virtual int visitEnum(hif::Enum &o);
    virtual int visitEnumValue(hif::EnumValue &o);
    virtual int visitExpression(hif::Expression &o);
    virtual int visitFunctionCall(hif::FunctionCall &o);
    virtual int visitField(hif::Field &o);
    virtual int visitFieldReference(hif::FieldReference &o);
    virtual int visitFile(hif::File &o);
    virtual int visitFor(hif::For &o);
    virtual int visitForGenerate(hif::ForGenerate &o);
    virtual int visitFunction(hif::Function &o);
    virtual int visitGlobalAction(hif::GlobalAction &o);
    virtual int visitEntity(hif::Entity &o);
    virtual int visitIdentifier(hif::Identifier &o);
    virtual int visitIf(hif::If &o);
    virtual int visitIfAlt(hif::IfAlt &o);
    virtual int visitIfGenerate(hif::IfGenerate &o);
    virtual int visitInstance(hif::Instance &o);
    virtual int visitInt(hif::Int &o);
    virtual int visitIntValue(hif::IntValue &o);
    virtual int visitLibraryDef(hif::LibraryDef &o);
    virtual int visitLibrary(hif::Library &o);
    virtual int visitMember(hif::Member &o);
    virtual int visitNull(hif::Null &o);
    virtual int visitTransition(hif::Transition &o);
    virtual int visitParameterAssign(hif::ParameterAssign &o);
    virtual int visitParameter(hif::Parameter &o);
    virtual int visitProcedureCall(hif::ProcedureCall &o);
    virtual int visitPointer(hif::Pointer &o);
    virtual int visitPortAssign(hif::PortAssign &o);
    virtual int visitPort(hif::Port &o);
    virtual int visitProcedure(hif::Procedure &o);
    virtual int visitRange(hif::Range &o);
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
    virtual int visitString(hif::String &o);
    virtual int visitStateTable(hif::StateTable &o);
    virtual int visitSystem(hif::System &o);
    virtual int visitSwitchAlt(hif::SwitchAlt &o);
    virtual int visitSwitch(hif::Switch &o);
    virtual int visitStringValue(hif::StringValue &o);
    virtual int visitTime(hif::Time &o);
    virtual int visitTimeValue(hif::TimeValue &o);
    virtual int visitTypeDef(hif::TypeDef &o);
    virtual int visitTypeReference(hif::TypeReference &o);
    virtual int visitTypeTPAssign(hif::TypeTPAssign &o);
    virtual int visitTypeTP(hif::TypeTP &o);
    virtual int visitUnsigned(hif::Unsigned &o);
    virtual int visitValueTPAssign(hif::ValueTPAssign &o);
    virtual int visitValueTP(hif::ValueTP &o);
    virtual int visitVariable(hif::Variable &o);
    virtual int visitView(hif::View &o);
    virtual int visitViewReference(hif::ViewReference &o);
    virtual int visitWait(hif::Wait &o);
    virtual int visitWhen(hif::When &o);
    virtual int visitWhenAlt(hif::WhenAlt &o);
    virtual int visitWhile(hif::While &o);
    virtual int visitWith(hif::With &o);
    virtual int visitWithAlt(hif::WithAlt &o);

private:
    VHDLPrinter(const VHDLPrinter &);
    VHDLPrinter &operator=(const VHDLPrinter &);

    /// @brief The reference semantics.
    hif::semantics::ILanguageSemantics *_sem;

    /// @brief The output directory where the VHDL output files are created.
    std::string _outDir;

    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_outstream;

    /// @brief The design unit name of the current subtree.
    std::string _currentDesignUnitName;

    /// @brief The view name of the current subtree.
    std::string _currentViewName;

    /// @brief Hif System of the current subtree.
    hif::System *_currentSystem;

    /// @brief Hif Contents of the current subtree.
    hif::Contents *_currentContents;

    /// @brief Store the components already printed.
    ViewMap _printedComponents;

    bool _isPrintComponents;

    bool _isRealRange;

    bool _isPrintWithCondition;

    bool _isPrintingLibDefDecls;

    bool _isSubProgramBody;

    /// @brief Create the new directory \p dirName.
    int _createDirectory(std::string dirName);

    /// @brief Create a file named \p fileName and initialize the output stream
    /// used to print the VHDL final description.
    void _initializeOutstream(std::string fileName, std::string subdirectory);

    /// @brief Print the comment of an HIF Object \p o.
    void _printComment(hif::Object *o);

    /// @brief Print the initial banner.
    void _printInitBanner();

    /// @brief Print the port direction corresponding string.
    void _printPortDirection(hif::PortDirection dir);

    /// @brief Print the libraries list of a DesignUnit.
    void _printLibraries(hif::BList<hif::Library> &libraries);

    /// @brief Special printing for file variables.
    bool _printFileVariable(hif::Variable *o);

    /// @brief Special printing for assert statement.
    bool _printAssertStatement(hif::ProcedureCall *o);

    void _printValueInstance(hif::Value *v);
    void _printTypeInstance(hif::ReferencedType *v);

    /// @name Print a list of objects.
    /// @param list The list to be printed.
    /// @param separator The separator among elements of \p list.
    /// @param needNewLine If true, a new line is added at the end of each
    /// element.
    /// @{

    template <class T>
    void _printList(hif::BList<T> &list, const char separator, const bool needNewLine);

    template <class T>
    void _printList(hif::BList<T> &list, const std::string &separator, const bool needNewLine);

    void _printList(hif::BList<hif::Object> &list, const std::string &separator, const bool needNewLine);

    /// @}

    void _setRealRange(hif::Range *o);
};
