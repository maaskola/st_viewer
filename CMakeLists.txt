project(STViewer CXX)
cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
set(APPLICATION_NAME "Spatial Transcriptomics Research Viewer")

# Configuration variables
set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "6")
set(PROJECT_VERSION_PATCH "2")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_DESCRIPTION "Visualization tool for Spatial Transcriptomics data")
set(PROJECT_COPYRIGHT "Copyright (C) 2016 Jose Fernandez Navarro, Jonas Frisen, Patrik Stall, Joakim Lundeberg, KTH, KI.")
set(PROJECT_CONTACT "jose.fernandez.navarro@scilifelab.se")
set(PROJECT_VENDOR "Jose Fernandez Navarro")

set(CONFIG_FILE "${PROJECT_SOURCE_DIR}/assets/stviewer.conf" CACHE STRING
    "The file with the network configuration settings")
set(COPY_SYSTEM_LIBRARIES false)

# print main variables
message(STATUS)
message(STATUS
"-------------------------------------------------------------------------------"
)
message(STATUS "Building ${APPLICATION_NAME} .....")
message(STATUS "Change a configuration variable with: cmake -D<Variable>=<Value>")
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "TARGET_ARCH = ${TARGET_ARCH}")
message(STATUS "VERSION = ${PROJECT_VERSION}")
message(STATUS "CONFIGURATION FILE = ${CONFIG_FILE}")
message(STATUS "COPY_SYSTEM_LIBRARIES = ${COPY_SYSTEM_LIBRARIES}")
message(STATUS
"-------------------------------------------------------------------------------"
)
message(STATUS)

enable_testing()

# Restrict possible builds to the selected build type.
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  if(WIN32)
    cmake_policy(SET CMP0020 NEW)
  endif(WIN32)
  if(APPLE)
    cmake_policy(SET CMP0025 NEW)
  endif()
endif(COMMAND cmake_policy)

# File that contains the default language for Qt
set(TRANSLATION_FILE "locale_en_us.qm")
# We just need to pass the app the name of the config file
get_filename_component(CONFIG_FILE_NAME ${CONFIG_FILE} NAME)

# Compile RC-file, only for Windows
if(WIN32)
  set (RC_FILE ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.rc)
  add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.o
    COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.rc
    -o ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.o)
endif (WIN32)

# Check whether the compiler supports position-independent code
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-fPIC" COMPILER_SUPPORTS_PIC)
if(COMPILER_SUPPORTS_PIC)
  add_definitions("-fPIC")
endif()

# Some in-house CMake functions/macros
include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)

# Check the the configuration file is present
if (NOT EXISTS ${CONFIG_FILE})
    message(FATAL_ERROR "Configuration file not present!")
endif()

# Compile CMake generated based files
configure_file(${PROJECT_SOURCE_DIR}/assets/application.qrc.in ${PROJECT_BINARY_DIR}/application.qrc)
configure_file(${PROJECT_SOURCE_DIR}/cmake/options_cmake.h.in ${PROJECT_BINARY_DIR}/options_cmake.h)

# Include the project binary dir
include_directories(${PROJECT_BINARY_DIR})

# Find the path to Qt binaries and set the Qt libraries
find_package(Qt5LinguistTools REQUIRED CONFIG)
set(ST_QT_LIBS  Widgets PrintSupport Network Core Gui OpenGL Concurrent)
set(ST_ICU_LIBS icuuc icui18n icudata)
if(UNIX AND NOT APPLE)
  list(APPEND ST_QT_LIBS DBus Svg)
endif()
foreach(i ${ST_QT_LIBS})
  use_qt5lib("Qt5${i}")
endforeach()

# Add cutom find.cmake files
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Initialize project settings (important to do it first as it defines compilation flags)
initialise_project()

IF(EXISTS "${CMAKE_SOURCE_DIR}/local_configuration.cmake")
  INCLUDE("${CMAKE_SOURCE_DIR}/local_configuration.cmake")
ENDIF()

add_subdirectory(ext) # include third party libraries
add_subdirectory(src) # include source code
