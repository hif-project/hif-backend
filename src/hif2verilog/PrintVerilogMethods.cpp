/// @file PrintVerilogMethods.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/backends/printFileMethods.hpp>

#include "hif2verilog/PrintVerilogMethods.hpp"
#include "hif2verilog/VerilogPrinter.hpp"

using namespace hif;

/// @brief Visitor to generate source files.
class VerilogVisitor : public hif::GuideVisitor
{
public:
    /// @brief Constructor.
    VerilogVisitor(hif2verilogParseLine &cLine);

    /// @brief Destructor.
    ~VerilogVisitor() override;

    VerilogVisitor(const VerilogVisitor &) = delete;

    auto operator=(const VerilogVisitor &) -> VerilogVisitor & = delete;

    /// @name Refinement methods.
    //@{

    auto visitDesignUnit(hif::DesignUnit &o) -> int override;

    auto visitLibraryDef(hif::LibraryDef &o) -> int override;

    auto visitSystem(hif::System &o) -> int override;

    //@}

private:
    /// @brief Print implementation.
    /// @param o The object to print.
    /// @param fileName The name of the file to write on (differenced basing
    /// on the kind of object).
    /// @param headerImplementation Used to distinguish between print of
    /// templated and non-templated components.
    void _printImplementation(hif::Object &o, const std::string& fileName);

    /// @brief The output stream to write on.
    hif::backends::IndentedStream *_outstream{nullptr};

    /// @brief The main output directory, used to create the directory hierarchy
    /// related to this HIF tree.
    std::string _outdirName;

    /// @brief The semantics
    hif::semantics::VerilogSemantics *_sem;

    /// @brief Variable to check if AMS extension is enabled.
    bool _ams_enabled{};
};

VerilogVisitor::VerilogVisitor(hif2verilogParseLine &cLine)
    : _outdirName(cLine.getOutputDirectory())
    , _sem(hif::semantics::VerilogSemantics::getInstance())
{
}

VerilogVisitor::~VerilogVisitor()
{
    hif::application_utils::restoreLogHeader();
    delete _outstream;
}

auto VerilogVisitor::visitDesignUnit(DesignUnit &o) -> int
{
    messageAssert(!o.views.empty() && o.views.size() == 1, "Unsupported more than one view", &o, nullptr);
    View *duView = o.views.front();

    // skip standard ones
    if (duView->isStandard()) {
        return 0;
    }

    if (hif::languageIDToString(o.views.at(0)->getLanguageID()) == "AMS") {
        _ams_enabled = true;
        auto *sys    = dynamic_cast<System *>(o.getParent());
        if (sys != nullptr) {
            sys->libraryDefs.push_back(_sem->getStandardLibrary("vams_standard"));
            // The following code is a workaround until the inclusion issue will be fixed in lexer
            sys->libraryDefs.push_back(_sem->getStandardLibrary("vams_constants"));
            sys->libraryDefs.push_back(_sem->getStandardLibrary("vams_disciplines"));
            sys->libraryDefs.push_back(_sem->getStandardLibrary("vams_driver_access"));
            hif::HifFactory f(_sem);
            sys->libraries.push_back(f.library("vams_standard", nullptr, "", false, true));
            sys->libraries.push_back(f.library("vams_constants", nullptr, "", false, true));
            sys->libraries.push_back(f.library("vams_disciplines", nullptr, "", false, true));
            sys->libraries.push_back(f.library("vams_driver_access", nullptr, "", false, true));
            _sem->addStandardPackages(sys);
        }
    }

    // If the DesignUnit contains components own TPs, print their implementation.
    std::string fileName;
    if (hif::languageIDToString(o.views.at(0)->getLanguageID()) == "AMS") {
        fileName = _outdirName + "/" + o.getName() + ".vams";
    } else {
        fileName = _outdirName + "/" + o.getName() + ".v";
    }

    _printImplementation(o, fileName);

    return 0;
}

auto VerilogVisitor::visitLibraryDef(LibraryDef &o) -> int
{
    // Standard libraries
    if (o.isStandard()) {
        return 0;
    }

    // Recursive call.
    GuideVisitor::visitLibraryDef(o);

    messageInfo("Verilog Visitor into LibraryDef.");
    return 0;
}

auto VerilogVisitor::visitSystem(System &o) -> int
{
    // Recursive call.
    GuideVisitor::visitSystem(o);
    messageInfo("Verilog Visitor into System.");
    return 0;
}

void VerilogVisitor::_printImplementation(Object &o, const std::string& fileName)
{
    if (fileName.empty()) {
        return;
    }

    if (_outstream != nullptr) {
        delete _outstream;
        _outstream = nullptr;
    }

    std::string f;
    std::string ext;
    hif::backends::splitFileName(fileName, f, ext);

    auto *lib = dynamic_cast<LibraryDef *>(&o);
    if (lib == nullptr) {
        std::ofstream ofs;
        if (hif::backends::openFileStream(fileName, &ofs) == 0) {
            messageError("Unable to create implementation output stream for " + fileName, nullptr, nullptr);
        }
        ofs.close();

        _outstream = new hif::backends::IndentedStream(f, ext);
        _outstream->setComment("// ", "// ", "");
    }

    *(_outstream) << "// @file " << fileName << "\n";
    *(_outstream) << "// @brief This file was generated by hif2verilog.\n";
    *(_outstream) << "// @details\n";
    *(_outstream) << "// Generate with HIF version " << hif::application_utils::getHIFVersion() << ".\n";
    *(_outstream) << "\n";
    if (_ams_enabled) {
        *(_outstream) << "`include \"constants.vams\"\n";
        *(_outstream) << "`include \"disciplines.vams\"\n";
        *(_outstream) << "\n";
    }

    VerilogPrinter vis(_outstream);
    o.acceptVisitor(vis);

    if (_outstream != nullptr) {
        *(_outstream) << '\n';
    }
}

void printVerilog(hif::System *sys, hif2verilogParseLine &cLine)
{
    messageAssert(sys != nullptr, "Given tree root is not valid", nullptr, nullptr);

    VerilogVisitor p(cLine);

    sys->acceptVisitor(p);
}
