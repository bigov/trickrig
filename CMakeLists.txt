#
# При вызове команды "cmake" (под win32) НЕОБХОДИМО указать тип сборки,
# например, для сборки в MinGW GCC указывается -
#
# -DCMAKE_BUILD_TYPE=Release ../ -G "MinGW Makefiles"
#
# Потом (для всех)
#   cmake --build .
#
#
# Под Linux можно собрать так:
#   cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Debug ../ && cmake --build . && ./app_dbg
#   cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Release ../ && cmake --build . && ./app_rel
#
# Сборка с использование Clang:
#   CC="clang" CXX="clang++" cmake ../ && cmake --build .
#
# https://cmake.org/Wiki/CMake_Useful_Variables
#
cmake_minimum_required( VERSION "3.15" )
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

project(
  TrickRig VERSION "0.0.2.00"
  DESCRIPTION "Graphical engine project"
  HOMEPAGE_URL "https://www.trickrig.net"
  LANGUAGES "CXX"
  )

if( "_$ENV{CC}" MATCHES "clang" )
  SET( CLANG TRUE )
  SET( CXX_FLAGS "${CXX_FLAGS} -pthread" )
else()
  SET( CLANG FALSE )
  SET( CXX_FLAGS "${CXX_FLAGS} -Wunreachable-code -latomic -lpthread" )
endif()

set(PROJECT_AUTHOR "bigov")
set(PROJECT_AUTHOR_EMAIL "bigov@yandex.ru")
SET( SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE )
SET( CMAKE_VERBOSE_MAKEFILE OFF )

configure_file( "${CMAKE_SOURCE_DIR}/cmakes/version_hpp.cmake" "${CMAKE_CURRENT_BINARY_DIR}/version.hpp" )

include_directories( SYSTEM "${CMAKE_SOURCE_DIR}/libs" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )
#include_directories( "${CMAKE_HOME_DIRECTORY}" )

# Для сборки релиза следует указать: "-D CMAKE_BUILD_TYPE=Release"
if( "_${CMAKE_BUILD_TYPE}" MATCHES "^_Rel" )
  OPTION( WITH_DEBUG_MODE "Build with debug mode" OFF )
  SET( CMAKE_BUILD_TYPE "Release")
else() # Иначе собирается "Debug"
  OPTION( WITH_DEBUG_MODE "Build with debug mode" ON )
  SET( CMAKE_BUILD_TYPE "Debug")
endif()

SET( CXX_FLAGS "-m64 -std=c++2a -fexceptions -Werror -Wpedantic -Wextra -Woverloaded-virtual \
  -Wctor-dtor-privacy -Wnon-virtual-dtor -Wall -Winit-self" ) ## -O3

SET( CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE )
SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie" )

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  SET( CXX_FLAGS "${CXX_FLAGS} -mwindows -Wl,-subsystem,windows" )
endif()

SET( CMAKE_CXX_FLAGS "${CXX_FLAGS}" )

find_package( PkgConfig REQUIRED )

if( ${MINGW} )
  SET( TR_LIBS mingw32 gdi32 )
endif()

# server
#include( "${CMAKE_SOURCE_DIR}/cmakes/server.cmake" )

# main
include( "${CMAKE_SOURCE_DIR}/cmakes/main.cmake" )

