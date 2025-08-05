cmake_minimum_required(VERSION 3.10)

##############################################################################################################################
# Vars and Versions:
set(SVERSION 2.8.32)

set(PROJECT_HOMEPAGE_URL "https://github.com/unmanarc/libMantids")
set(PROJECT_CONTACT "Aaron G. Mizrachi P. <aaron@unmanarc.com>")
##############################################################################################################################

##############################################################################################################################
# C++ Global Options Settings:
set(CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# TODO: check using CLANG:
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -pedantic -Weverything -Wno-c++98-compat -Wno-switch-default -Wno-padded -Wno-documentation -Wno-unused-macros -Wno-undef -Wno-unused-parameters -Wno-sign-conversion -Wno-unsafe-buffer-usage -Wdeprecated -Wconversion -Wshadow")

option(BUILD_SHARED_LIBS "Enable building the library as a shared library instead of a static one." ON)
# Create an option to toggle between C++11 and C++17 for regex handling
option(USE_STD_REGEX "Use std::regex from C++17 if Boost regex is not available" OFF)

##############################################################################################################################

##############################################################################################################################
# Project Settings:
project(libMantids)
project(${PROJECT_NAME} VERSION ${SVERSION} DESCRIPTION "Mini-Advanced Network Toolkit for Internet Services Development")
##############################################################################################################################

##############################################################################################################################
# Other CMake Global settings:
include(GNUInstallDirs)
##############################################################################################################################

#############################################################################################################################
# Subprojects:l
ADD_SUBDIRECTORY(Helpers)
ADD_SUBDIRECTORY(Scripts)
ADD_SUBDIRECTORY(Threads)
ADD_SUBDIRECTORY(Filesystem)
ADD_SUBDIRECTORY(Memory)
ADD_SUBDIRECTORY(Database)
ADD_SUBDIRECTORY(Network)
ADD_SUBDIRECTORY(Protocols)
ADD_SUBDIRECTORY(Auth)
ADD_SUBDIRECTORY(Program)
ADD_SUBDIRECTORY(RPC)
#ADD_SUBDIRECTORY(devel)
ADD_SUBDIRECTORY(extras)
#############################################################################################################################

#############################################################################################################################
# Update version in spec.:

# File where %define version is located
set(SPEC_FILE "${CMAKE_SOURCE_DIR}/rpm/build.spec")

# Create a temporary file
file(WRITE ${CMAKE_BINARY_DIR}/temp_build_spec.txt "")

# Read the original file line by line
file(STRINGS ${SPEC_FILE} SPEC_LINES)

foreach(LINE ${SPEC_LINES})
    # Find the line containing %define version and replace it with SVERSION value
    if("${LINE}" MATCHES "^%define version .*$")
        file(APPEND ${CMAKE_BINARY_DIR}/temp_build_spec.txt "%define version ${SVERSION}\n")
    else()
        file(APPEND ${CMAKE_BINARY_DIR}/temp_build_spec.txt "${LINE}\n")
    endif()
endforeach()

# Replace the original file with the temporary one
file(RENAME ${CMAKE_BINARY_DIR}/temp_build_spec.txt ${SPEC_FILE})
