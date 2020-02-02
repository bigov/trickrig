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
#project( trickrig )

#project(<PROJECT-NAME>
#        [VERSION <major>[.<minor>[.<patch>[.<tweak>]]]]
#        [LANGUAGES <language-name>...])

project(
  TrickRig VERSION "0.0.1.15"
  DESCRIPTION "Graphical engine project"
  HOMEPAGE_URL "https://www.trickrig.net"
  LANGUAGES "CXX"
  )

set(PROJECT_AUTHOR "bigov")
set(PROJECT_AUTHOR_EMAIL "bigov@yandex.ru")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

## Пока работаем без установки программы
SET( SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE )

SET( CMAKE_VERBOSE_MAKEFILE OFF )

configure_file("${CMAKE_SOURCE_DIR}/cmakes/version_hpp.cmake" "${CMAKE_CURRENT_BINARY_DIR}/version.hpp")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

SET( trCMAKES "${CMAKE_SOURCE_DIR}/cmakes" )
SET( trEXT_LIBS_DIR "${CMAKE_SOURCE_DIR}/3rdlibs" )
SET( trINCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include" )

# список исходников
file( GLOB trSRC "${CMAKE_SOURCE_DIR}/src/*.cpp" )

include( "${trCMAKES}/main.cmake" )

