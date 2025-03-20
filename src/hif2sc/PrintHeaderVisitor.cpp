/// @file PrintHeaderVisitor.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <hif/backends/printFileMethods.hpp>

#include "hif2sc/PrintMethods.hpp"

using std::endl;
using std::ofstream;
using std::string;
using std::vector;
using namespace hif;

namespace
{

// /////////////////////////////////////////////////////////////////////////////
// PrintHeaderVisitor
// /////////////////////////////////////////////////////////////////////////////

/// @brief Visitor to generate header files.
class PrintHeaderVisitor : public hif::GuideVisitor
{

public:
    /// @brief Constructor.
    PrintHeaderVisitor(hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList);

    /// @brief Destructor.
    virtual ~PrintHeaderVisitor();

    /// @name Refinement methods.
    //@{

    virtual int visitDesignUnit(hif::DesignUnit &o);
    virtual int visitLibraryDef(hif::LibraryDef &o);
    virtual int visitSystem(hif::System &o);

    //@}

private:
    PrintHeaderVisitor(const PrintHeaderVisitor &);
    PrintHeaderVisitor &operator=(const PrintHeaderVisitor &);

    /// @name Functions that print a new header file for the component.
    //@{

    int _printHeader(hif::DesignUnit &o);
    int _printHeader(hif::LibraryDef &o);
    int _printHeader(hif::System &o);
    int _printHeader(hif::Object &o, string fileName);

    //@}

    int _createDirectory(const string &dirName);
    int _createDirectories(
        const string &outdirName,
        const std::string &libDefName,
        PrintSystemCVisitor::ConstTemplateMap &ctmList);
    std::string _getHeaderExtensionByLanguage(Object *obj);
    std::string
    _getFileName(const std::string &outDir, const std::string &libDefName, const std::string &fileName, Object *obj);

    /// @brief The output stream to write on.
    ///
    hif::backends::IndentedStream *_outstream;

    /// @brief The main output directory, used to create the directory hierarchy
    /// related to this HIF tree.
    ///
    string _outdirName;

    /// @name The fields related to nested components.
    //@{

    /// @brief When printing a LibraryDef, all its contents will be included in
    /// a directory with its same name. This fields is set in visit of LibraryDef,
    /// and it is used to recognize nested components (if any).
    hif::LibraryDef *_currentLibraryDef;

    /// @brief This fields is set in visit of DesignUnit, and it is used to
    /// recognize nested components (if any).
    std::string _DUName;

    /// @brief Set to true, indicates the presence of global declarations.
    bool _includeSystemGlobals;

    //@}

    /// @name Command-line flags of hif2sc
    //@{

    bool _useResolved;
    bool _useHDTLib;
    const uint64_t _maxLines;
    const std::string _sourcesExtension;
    const std::string _headersExtension;

    //@}

    /// @brief The map of constant template.
    PrintSystemCVisitor::ConstTemplateMap &_ctmList;

    /// @brief The semantics
    hif::semantics::SystemCSemantics *_sem;
};

PrintHeaderVisitor::PrintHeaderVisitor(hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList)
    : _outstream(nullptr)
    , _outdirName(cLine.getOutputDirectory())
    , _currentLibraryDef(nullptr)
    , _DUName()
    , _includeSystemGlobals(false)
    , _useResolved(cLine.useResolved())
    , _useHDTLib(cLine.useHDTLib())
    , _maxLines(cLine.getMaxLines())
    , _sourcesExtension(cLine.getSourcesExtension())
    , _headersExtension(cLine.getHeadersExtension())
    , _ctmList(ctmList)
    , _sem(hif::semantics::SystemCSemantics::getInstance())
{
    hif::application_utils::initializeLogHeader("HIF2SC", "PrintHeaderVisitor");
}

PrintHeaderVisitor::~PrintHeaderVisitor()
{
    hif::application_utils::restoreLogHeader();
    delete _outstream;
}

int PrintHeaderVisitor::visitDesignUnit(DesignUnit &o)
{
    auto restore = o.getName();
    _DUName      = o.getName();

    _printHeader(o);

    _DUName = restore;
    return 0;
}

int PrintHeaderVisitor::visitLibraryDef(LibraryDef &o)
{
    // Skip standard libraries (e.g., tlm)
    if (o.isStandard())
        return 0;

    // Note: even if LibraryDef contains only DesignUnits or other LibraryDef,
    // the file is still generated to preserve the hierarchical structure.

    LibraryDef *restore = _currentLibraryDef;
    _currentLibraryDef  = &o;

    _printHeader(o);

    // Recursive call.
    GuideVisitor::visitLibraryDef(o);

    _currentLibraryDef = restore;
    return 0;
}

int PrintHeaderVisitor::visitSystem(System &o)
{
    if (!_printHeader(o))
        _includeSystemGlobals = true;

    // Recursive call.
    GuideVisitor::visitSystem(o);
    return 0;
}

int PrintHeaderVisitor::_printHeader(DesignUnit &o)
{
    // Normally there's only one View for each DesignUnit.
    messageAssert(!o.views.empty() && o.views.size() == 1, "Unsupported more than one view", &o, nullptr);
    View *duView = o.views.front();

    // skip standard ones
    if (duView->isStandard())
        return 0;

    // Feedback message
    messageInfo(string("Generating ") + getLanguage(duView->getLanguageID()) + " code for unit " + o.getName() + ".");

    // Note: directories have already been created.
    std::string ldName;
    if (_currentLibraryDef != nullptr) {
        ldName = _currentLibraryDef->getName();
    }
    string fileName = _getFileName(_outdirName, ldName, _DUName, duView);
    _printHeader(o, fileName);

    return 0;
}

int PrintHeaderVisitor::_printHeader(LibraryDef &o)
{
    std::string ldName;
    if (_currentLibraryDef != nullptr) {
        ldName = _currentLibraryDef->getName();
    }

    // Create the directories associated to this library.
    if (!_createDirectories(_outdirName, ldName, _ctmList)) {
        messageError("Error creating output directories.", nullptr, nullptr);
    }

    // Feedback message
    messageInfo(string("Generating ") + getLanguage(o.getLanguageID()) + " code for library " + o.getName() + ".");

    string fileName = _getFileName(_outdirName, ldName, ldName, &o);
    _printHeader(o, fileName);

    return 0;
}

int PrintHeaderVisitor::_printHeader(System &o)
{
    if (!_createDirectories(_outdirName, "", _ctmList)) {
        messageDebugAssert(false, "Unexpected case", nullptr, nullptr);
        messageError("Error creating output directories.", nullptr, nullptr);
    }

    if (o.declarations.empty() && o.actions.empty())
        return 1;

    // Feedback message
    messageInfo(string("Generating ") + getLanguage(o.getLanguageID()) + " code for global declarations.");

    string fileName = _getFileName(_outdirName, nullptr, NameTable::getInstance()->hifGlobals(), &o);
    _printHeader(o, fileName);

    return 0;
}

int PrintHeaderVisitor::_printHeader(Object &o, string fileName)
{
    ofstream actualStream;
    if (!hif::backends::openFileStream(fileName.data(), &actualStream)) {
        messageError(string("Error writing file ") + fileName, nullptr, nullptr);
    }
    actualStream.close();

    if (_outstream != nullptr) {
        delete _outstream;
        _outstream = nullptr;
    }

    std::string f;
    std::string ext;
    hif::backends::splitFileName(fileName, f, ext);
    _outstream = new hif::backends::IndentedStream(f, ext);
    _outstream->setComment("// ", "// ", "");

    PrintSystemCVisitorOptions opt;
    opt.printImplementation = false;
    opt.useResolved         = _useResolved;
    opt.useHDTLib           = _useHDTLib;
    opt.maxLines            = _maxLines;
    opt.sourcesExtension    = _sourcesExtension;
    opt.headersExtension    = _headersExtension;

    PrintSystemCVisitor vis(_outstream, opt, _ctmList, f, ext);
    vis.setCurrentLibraryDef(_currentLibraryDef);
    o.acceptVisitor(vis);

    *(_outstream) << endl; // Flush the stream

    return 0;
}

int PrintHeaderVisitor::_createDirectory(const string &dirName)
{
    hif::application_utils::FileStructure dir(dirName);
    // Empty directory if it already exists.
    if (dir.exists()) {
        vector<string> fileList = dir.list();
        for (vector<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
            hif::application_utils::FileStructure fileIn(*it);
            fileIn.rmfile_weak();
        }
    }
    // Create new directory (if it does not already exist)
    else if (!dir.make_dir()) {
        messageError("Directory generation not successful.", nullptr, nullptr);
    }

    return 1;
}

int PrintHeaderVisitor::_createDirectories(
    const std::string &outdirName,
    const std::string &libDefName,
    PrintSystemCVisitor::ConstTemplateMap &ctmList)
{
    PrintSystemCVisitor vis(ctmList);
    // To portability under Windows, directory paths must not end with a trailing '/' character.
    // Thus, we manage the concatenation directly in ld var.
    string ld = (libDefName.empty()) ? "" : "/" + std::string(libDefName);

    string dirName = outdirName + "/inc" + ld;
    if (!_createDirectory(dirName))
        return 0;

    dirName = outdirName + "/src" + ld;
    if (!_createDirectory(dirName))
        return 0;

    return 1;
}

std::string PrintHeaderVisitor::_getHeaderExtensionByLanguage(Object *obj)
{
    const hif::LanguageID lang = objectGetLanguage(obj);
    LibraryDef *ld             = dynamic_cast<LibraryDef *>(obj);

    if (lang == hif::c || (ld != nullptr && ld->getLanguageID() == hif::cpp && ld->hasCLinkage()))
        return ".h";

    // rtl, tlm, psl, cpp
    return "." + _headersExtension;
}

std::string PrintHeaderVisitor::_getFileName(
    const std::string &outDir,
    const std::string &libDefName,
    const std::string &fileName,
    Object *obj)
{
    string ret("");
    string ld = (libDefName.empty()) ? "" : std::string(libDefName) + "/";
    string fn = (fileName.empty()) ? "" : fileName;

    ret = outDir + "/inc/" + ld + fn + _getHeaderExtensionByLanguage(obj);
    return ret;
}

} // namespace

void printHeaders(hif::System *sys, hif2scParseLine &cLine, PrintSystemCVisitor::ConstTemplateMap &ctmList)
{
    PrintHeaderVisitor writeHeader(cLine, ctmList);
    sys->acceptVisitor(writeHeader);
}
