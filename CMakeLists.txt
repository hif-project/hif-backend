# -----------------------------------------------------------------------------
# @brief  : HIF cmake file.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

# Set the minimum CMake version, the project name and default build type.
cmake_minimum_required(VERSION 3.1...3.18)

# Set the project name.
project(backend CXX)

# Set the default build type to Debug.
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# Set the install prefix.
set(CMAKE_INSTALL_PREFIX /usr/local)

# Set modules path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# -----------------------------------------------------------------------------
# OPTIONS
# -----------------------------------------------------------------------------

option(STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)

# -----------------------------------------------------------------------------
# ENABLE FETCH CONTENT
# -----------------------------------------------------------------------------

# We need this in order to import external projects.
include(FetchContent)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_QUIET
    FETCHCONTENT_BASE_DIR
    FETCHCONTENT_FULLY_DISCONNECTED
    FETCHCONTENT_UPDATES_DISCONNECTED
)

# -----------------------------------------------------------------------------
# DEPENDENCY (SYSTEM LIBRARIES)
# -----------------------------------------------------------------------------

find_package(Doxygen)
find_package(SystemCLanguage CONFIG REQUIRED)
find_package(HIF REQUIRED)
find_program(CLANG_TIDY_EXE NAMES clang-tidy)

# -----------------------------------------------------------------------------
# EXECUTABLEs
# -----------------------------------------------------------------------------

# =====================================
# EXECUTABLE: hif2sc
# =====================================
# Create the executable.
add_executable(
    hif2sc
    ${CMAKE_SOURCE_DIR}/src/hif2sc/globals.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/hif2sc.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/hif2scParseLine.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PreRefine_conflictingSubPrograms.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PreRefine_misc.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PreRefine_utilityLibraries.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PostRefine_cpp98.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PostRefine_optimization.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PostRefine_final.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PrintHeaderVisitor.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PrintImplementationVisitor.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PrintMethods.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PrintSystemCVisitor.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2sc/PrintRefinementsVisitor.cpp
)
# Include directories.
target_include_directories(hif2sc PRIVATE ${CMAKE_SOURCE_DIR}/include ${HIF_INCLUDE_DIR})
# Link libraries.
target_link_libraries(hif2sc PRIVATE ${HIF_LIBRARY})
# Compilation options.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings for MSVC-specific "safe" functions like strcpy_s, etc.,
    # which are not portable and may clutter warning logs.
    target_compile_definitions(hif2sc PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Disable warning C4702: unreachable code.
    add_compile_options(/wd4702)
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2sc PRIVATE /WX)
    endif()
    if(STRICT_WARNINGS)
        # Enable external header management to suppress warnings in system and
        # external headers, making it easier to focus on project-specific issues.
        target_compile_options(hif2sc PRIVATE /experimental:external)
        target_compile_options(hif2sc PRIVATE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(hif2sc PRIVATE /external:anglebrackets)
        target_compile_options(hif2sc PRIVATE /external:W0)

        # Use a high warning level to catch as many potential issues as possible.
        target_compile_options(hif2sc PRIVATE /W4)

        # Enforce standards-compliant behavior to avoid relying on MSVC-specific extensions.
        target_compile_options(hif2sc PRIVATE /permissive-)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2sc PRIVATE -Werror)
    endif()
    if(STRICT_WARNINGS)
        # Enable a broad set of warnings to catch common and subtle issues:
        target_compile_options(hif2sc PRIVATE
            -Wall                # Enable most general-purpose warnings.
            -Wextra              # Enable extra warnings not included in -Wall.
            -Wconversion         # Warn about implicit type conversions that may lose data.
            -pedantic            # Enforce strict compliance with the C++ standard.
            -Wshadow             # Warn about variable shadowing, which can cause subtle bugs.
            -Wnon-virtual-dtor   # Warn when a class with virtual functions lacks a virtual destructor.
            -Wnull-dereference   # Warn about potential null pointer dereferences.
            -Wformat=2           # Enable strict checks for printf/scanf format strings.
            -Woverloaded-virtual # Warn when a derived class function hides a base class virtual function.
            -Wfloat-equal        # Warn about direct comparisons of floating-point values, which can be imprecise.
        )
    endif()
endif()
# Install the executable.
install(TARGETS hif2sc DESTINATION bin)

# =====================================
# EXECUTABLE: hif2verilog
# =====================================
# Create the executable.
add_executable(
    hif2verilog
    ${CMAKE_SOURCE_DIR}/src/hif2verilog/hif2verilog.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2verilog/hif2verilogParseLine.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2verilog/PrintVerilogMethods.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2verilog/VerilogPrinter.cpp
)
# Include directories.
target_include_directories(hif2verilog PRIVATE ${CMAKE_SOURCE_DIR}/include ${HIF_INCLUDE_DIR})
# Link libraries.
target_link_libraries(hif2verilog PRIVATE ${HIF_LIBRARY})
# Compilation options.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings for MSVC-specific "safe" functions like strcpy_s, etc.,
    # which are not portable and may clutter warning logs.
    target_compile_definitions(hif2verilog PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Disable warning C4702: unreachable code.
    add_compile_options(/wd4702)
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2verilog PRIVATE /WX)
    endif()
    if(STRICT_WARNINGS)
        # Enable external header management to suppress warnings in system and
        # external headers, making it easier to focus on project-specific issues.
        target_compile_options(hif2verilog PRIVATE /experimental:external)
        target_compile_options(hif2verilog PRIVATE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(hif2verilog PRIVATE /external:anglebrackets)
        target_compile_options(hif2verilog PRIVATE /external:W0)

        # Use a high warning level to catch as many potential issues as possible.
        target_compile_options(hif2verilog PRIVATE /W4)

        # Enforce standards-compliant behavior to avoid relying on MSVC-specific extensions.
        target_compile_options(hif2verilog PRIVATE /permissive-)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2verilog PRIVATE -Werror)
    endif()
    if(STRICT_WARNINGS)
        # Enable a broad set of warnings to catch common and subtle issues:
        target_compile_options(hif2verilog PRIVATE
            -Wall                # Enable most general-purpose warnings.
            -Wextra              # Enable extra warnings not included in -Wall.
            -Wconversion         # Warn about implicit type conversions that may lose data.
            -pedantic            # Enforce strict compliance with the C++ standard.
            -Wshadow             # Warn about variable shadowing, which can cause subtle bugs.
            -Wnon-virtual-dtor   # Warn when a class with virtual functions lacks a virtual destructor.
            -Wnull-dereference   # Warn about potential null pointer dereferences.
            -Wformat=2           # Enable strict checks for printf/scanf format strings.
            -Woverloaded-virtual # Warn when a derived class function hides a base class virtual function.
            -Wfloat-equal        # Warn about direct comparisons of floating-point values, which can be imprecise.
        )
    endif()
endif()
# Install the executable.
install(TARGETS hif2verilog DESTINATION bin)

# =====================================
# EXECUTABLE: hif2vhdl
# =====================================
# Create the executable.
add_executable(
    hif2vhdl
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/hif2vhdl.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/hif2vhdlParseLine.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/PreRefine_utilityLibraries.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/PrintMethods.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/VHDLPrinter.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/VhdlPostRefine_final.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/PreRefine_identifierNames.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/PreRefine_readOutPorts.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2vhdl/VhdlPreRefine_misc.cpp
)
# Include directories.
target_include_directories(hif2vhdl PRIVATE ${CMAKE_SOURCE_DIR}/include ${HIF_INCLUDE_DIR})
# Link libraries.
target_link_libraries(hif2vhdl PRIVATE ${HIF_LIBRARY})
# Compilation options.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings for MSVC-specific "safe" functions like strcpy_s, etc.,
    # which are not portable and may clutter warning logs.
    target_compile_definitions(hif2vhdl PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Disable warning C4702: unreachable code.
    add_compile_options(/wd4702)
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2vhdl PRIVATE /WX)
    endif()
    if(STRICT_WARNINGS)
        # Enable external header management to suppress warnings in system and
        # external headers, making it easier to focus on project-specific issues.
        target_compile_options(hif2vhdl PRIVATE /experimental:external)
        target_compile_options(hif2vhdl PRIVATE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(hif2vhdl PRIVATE /external:anglebrackets)
        target_compile_options(hif2vhdl PRIVATE /external:W0)

        # Use a high warning level to catch as many potential issues as possible.
        target_compile_options(hif2vhdl PRIVATE /W4)

        # Enforce standards-compliant behavior to avoid relying on MSVC-specific extensions.
        target_compile_options(hif2vhdl PRIVATE /permissive-)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2vhdl PRIVATE -Werror)
    endif()
    if(STRICT_WARNINGS)
        # Enable a broad set of warnings to catch common and subtle issues:
        target_compile_options(hif2vhdl PRIVATE
            -Wall                # Enable most general-purpose warnings.
            -Wextra              # Enable extra warnings not included in -Wall.
            -Wconversion         # Warn about implicit type conversions that may lose data.
            -pedantic            # Enforce strict compliance with the C++ standard.
            -Wshadow             # Warn about variable shadowing, which can cause subtle bugs.
            -Wnon-virtual-dtor   # Warn when a class with virtual functions lacks a virtual destructor.
            -Wnull-dereference   # Warn about potential null pointer dereferences.
            -Wformat=2           # Enable strict checks for printf/scanf format strings.
            -Woverloaded-virtual # Warn when a derived class function hides a base class virtual function.
            -Wfloat-equal        # Warn about direct comparisons of floating-point values, which can be imprecise.
        )
    endif()
endif()
# Install the executable.
install(TARGETS hif2vhdl DESTINATION bin)

# =====================================
# LIBARY: hif2scSupport
# =====================================
# Create the executable.
add_library(
    hif2scSupport
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_after.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_assign.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_systemc_extensions.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_verilog_standard.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_ieee_numeric_std.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_ieee_math_real.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_ieee_std_logic_1164.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_ieee_std_logic_arith.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_ieee_std_logic_signed.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_ieee_std_logic_textio.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_ieee_std_logic_unsigned.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_standard.cpp
    ${CMAKE_SOURCE_DIR}/src/hif2scSupport/hif_vhdl_std_textio.cpp
)
# Include directories.
target_include_directories(hif2scSupport PRIVATE ${CMAKE_SOURCE_DIR}/include ${HIF_INCLUDE_DIR})
# Link libraries.
target_link_libraries(hif2scSupport PRIVATE SystemC::systemc ${HIF_LIBRARY})
# Compilation options.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings for MSVC-specific "safe" functions like strcpy_s, etc.,
    # which are not portable and may clutter warning logs.
    target_compile_definitions(hif2scSupport PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Disable warning C4702: unreachable code.
    add_compile_options(/wd4702)
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2scSupport PRIVATE /WX)
    endif()
    if(STRICT_WARNINGS)
        # Enable external header management to suppress warnings in system and
        # external headers, making it easier to focus on project-specific issues.
        target_compile_options(hif2scSupport PRIVATE /experimental:external)
        target_compile_options(hif2scSupport PRIVATE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(hif2scSupport PRIVATE /external:anglebrackets)
        target_compile_options(hif2scSupport PRIVATE /external:W0)

        # Use a high warning level to catch as many potential issues as possible.
        target_compile_options(hif2scSupport PRIVATE /W4)

        # Enforce standards-compliant behavior to avoid relying on MSVC-specific extensions.
        target_compile_options(hif2scSupport PRIVATE /permissive-)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(hif2scSupport PRIVATE -Werror)
    endif()
    if(STRICT_WARNINGS)
        # Enable a broad set of warnings to catch common and subtle issues:
        target_compile_options(hif2scSupport PRIVATE
            -Wall                # Enable most general-purpose warnings.
            -Wextra              # Enable extra warnings not included in -Wall.
            -Wconversion         # Warn about implicit type conversions that may lose data.
            -pedantic            # Enforce strict compliance with the C++ standard.
            -Wshadow             # Warn about variable shadowing, which can cause subtle bugs.
            -Wnon-virtual-dtor   # Warn when a class with virtual functions lacks a virtual destructor.
            -Wnull-dereference   # Warn about potential null pointer dereferences.
            -Wformat=2           # Enable strict checks for printf/scanf format strings.
            -Woverloaded-virtual # Warn when a derived class function hides a base class virtual function.
            -Wfloat-equal        # Warn about direct comparisons of floating-point values, which can be imprecise.
        )
    endif()
endif()
# Install the executable.
install(TARGETS hif2scSupport DESTINATION bin)
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/hif2scSupport
    DESTINATION include
    COMPONENT header_files
    FILES_MATCHING PATTERN "*.hpp"
)

# -----------------------------------------------------------------------------
# CODE ANALYSIS
# -----------------------------------------------------------------------------

if(CLANG_TIDY_EXE)
    file(GLOB_RECURSE ALL_PROJECT_FILES
        "${PROJECT_SOURCE_DIR}/include/**/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/**/*.hh"
        "${PROJECT_SOURCE_DIR}/include/**/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/*.hh"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cc"
        "${PROJECT_SOURCE_DIR}/src/**/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.c"
    )
    add_custom_target(
        ${PROJECT_NAME}_clang_tidy
        COMMAND ${CLANG_TIDY_EXE}
        --p=${CMAKE_BINARY_DIR}
        ${ALL_PROJECT_FILES}
        COMMENT "Running clang-tidy"
        VERBATIM
    )
    add_custom_target(
        ${PROJECT_NAME}_clang_tidy_fix
        COMMAND ${CLANG_TIDY_EXE}
        --fix --fix-errors
        --p=${CMAKE_BINARY_DIR}
        ${ALL_PROJECT_FILES}
        COMMENT "Running clang-tidy-fix"
        VERBATIM
    )
endif()

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
        
    # FetchContent: Doxygen Awesome CSS
    FetchContent_Declare(doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG main
    )
    FetchContent_MakeAvailable(doxygenawesome)

    # Hide FetchContent variables to avoid clutter in ccmake.
    mark_as_advanced(FORCE
        FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
        FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
    )

    # Customization: Doxygen Configuration
    set(DOXYGEN_WARN_FORMAT "$file:$line:1: $text")
    set(DOXYGEN_PROJECT_NAME "HIF Backends")
    set(DOXYGEN_PROJECT_BRIEF "The HIF backends.")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_GENERATE_LATEX NO) # Disable LaTeX if not needed
    set(DOXYGEN_GENERATE_MAN NO)   # Disable man pages if not needed

    # Styling and UX enhancements using Doxygen Awesome
    set(DOXYGEN_HTML_HEADER ${doxygenawesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )

    # Set stricter warnings for better documentation quality
    set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
    set(DOXYGEN_WARN_IF_DOC_ERROR YES)
    set(DOXYGEN_WARN_NO_PARAMDOC YES)
    set(DOXYGEN_WARN_AS_ERROR NO) # Treat warnings as errors for CI

    # Add Doxygen documentation target.
    file(GLOB_RECURSE ALL_PROJECT_FILES
        "${PROJECT_SOURCE_DIR}/include/**/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/**/*.hh"
        "${PROJECT_SOURCE_DIR}/include/**/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/*.hh"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cc"
        "${PROJECT_SOURCE_DIR}/src/**/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.c"
    )

    doxygen_add_docs(
        ${PROJECT_NAME}_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md
        ${ALL_PROJECT_FILES}
        COMMENT "Generating Doxygen documentation for the library"
    )

endif()