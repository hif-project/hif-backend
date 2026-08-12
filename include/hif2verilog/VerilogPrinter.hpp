/// @file VerilogPrinter.hpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#pragma once

#include <fstream>

#include <hif/hif.hpp>

class VerilogPrinter : public hif::GuideVisitor
{
public:
    using ViewSet = std::set<hif::View *>;
    using ViewMap = std::map<hif::View *, ViewSet>;

    VerilogPrinter(hif::backends::IndentedStream *stream);
    ~VerilogPrinter() override;

    VerilogPrinter(const VerilogPrinter &)                     = delete;
    auto operator=(const VerilogPrinter &) -> VerilogPrinter & = delete;

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
    auto visitEnum(hif::Enum &o) -> int override;
    auto visitEnumValue(hif::EnumValue &o) -> int override;
    auto visitEvent(hif::Event &o) -> int override;
    auto visitExpression(hif::Expression &o) -> int override;
    auto visitFunctionCall(hif::FunctionCall &o) -> int override;
    auto visitField(hif::Field &o) -> int override;
    auto visitFieldReference(hif::FieldReference &o) -> int override;
    auto visitFile(hif::File &o) -> int override;
    auto visitFor(hif::For &o) -> int override;
    auto visitForGenerate(hif::ForGenerate &o) -> int override;
    auto visitFunction(hif::Function &o) -> int override;
    auto visitGlobalAction(hif::GlobalAction &o) -> int override;
    auto visitEntity(hif::Entity &o) -> int override;
    auto visitIdentifier(hif::Identifier &o) -> int override;
    auto visitIf(hif::If &o) -> int override;
    auto visitIfAlt(hif::IfAlt &o) -> int override;
    auto visitIfGenerate(hif::IfGenerate &o) -> int override;
    auto visitInstance(hif::Instance &o) -> int override;
    auto visitInt(hif::Int &o) -> int override;
    auto visitIntValue(hif::IntValue &o) -> int override;
    auto visitLibraryDef(hif::LibraryDef &o) -> int override;
    auto visitLibrary(hif::Library &o) -> int override;
    auto visitMember(hif::Member &o) -> int override;
    auto visitNull(hif::Null &o) -> int override;
    auto visitTransition(hif::Transition &o) -> int override;
    auto visitParameterAssign(hif::ParameterAssign &o) -> int override;
    auto visitParameter(hif::Parameter &o) -> int override;
    auto visitProcedureCall(hif::ProcedureCall &o) -> int override;
    auto visitPointer(hif::Pointer &o) -> int override;
    auto visitPortAssign(hif::PortAssign &o) -> int override;
    auto visitPort(hif::Port &o) -> int override;
    auto visitProcedure(hif::Procedure &o) -> int override;
    auto visitRange(hif::Range &o) -> int override;
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
    auto visitString(hif::String &o) -> int override;
    auto visitStateTable(hif::StateTable &o) -> int override;
    auto visitSystem(hif::System &o) -> int override;
    auto visitSwitchAlt(hif::SwitchAlt &o) -> int override;
    auto visitSwitch(hif::Switch &o) -> int override;
    auto visitStringValue(hif::StringValue &o) -> int override;
    auto visitTime(hif::Time &o) -> int override;
    auto visitTimeValue(hif::TimeValue &o) -> int override;
    auto visitTypeDef(hif::TypeDef &o) -> int override;
    auto visitTypeReference(hif::TypeReference &o) -> int override;
    auto visitTypeTPAssign(hif::TypeTPAssign &o) -> int override;
    auto visitTypeTP(hif::TypeTP &o) -> int override;
    auto visitUnsigned(hif::Unsigned &o) -> int override;
    auto visitValueStatement(hif::ValueStatement &o) -> int override;
    auto visitValueTPAssign(hif::ValueTPAssign &o) -> int override;
    auto visitValueTP(hif::ValueTP &o) -> int override;
    auto visitVariable(hif::Variable &o) -> int override;
    auto visitView(hif::View &o) -> int override;
    auto visitViewReference(hif::ViewReference &o) -> int override;
    auto visitWait(hif::Wait &o) -> int override;
    auto visitWhen(hif::When &o) -> int override;
    auto visitWhenAlt(hif::WhenAlt &o) -> int override;
    auto visitWhile(hif::While &o) -> int override;
    auto visitWith(hif::With &o) -> int override;
    auto visitWithAlt(hif::WithAlt &o) -> int override;

private:
    /// @brief The reference semantics.
    hif::semantics::ILanguageSemantics *_sem;
    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_stream;

    std::string getDeclaration(hif::Declaration *declaration);

    std::string getBitwidth(hif::Type *type);

    std::string getValue(hif::Value *value);

    std::string getPortAssign(hif::PortAssign *port_assign);

    /// @brief Print a list of objects.
    /// @param list The list to be printed.
    /// @param separator The separator among elements of \p list.
    /// @param print_new_line If true, a new line is added at the end of each.
    /// @param print_last_separator When print_new_line is true, if true, the separator is printed also after the last element.
    void printList(
        const hif::BList<hif::Object> &list,
        const std::string &separator,
        bool print_new_line       = false,
        bool print_last_separator = false);

    /// @brief Calls the visitor on any Function inside the list.
    /// @param list The list of objects to be printed.
    /// @return true if any function was found, false otherwise.
    bool printFunctions(const hif::BList<hif::Object> &list);
};
