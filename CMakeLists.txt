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
#
# cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Debug ../ && cmake --build . && ./app_dbg
# cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Release ../ && cmake --build . && ./app_rel
#
# https://cmake.org/Wiki/CMake_Useful_Variables
#

cmake_minimum_required( VERSION 3.0 )
project( trickrig )
SET( CMAKE_VERBOSE_MAKEFILE OFF )

SET( trCMAKES "${CMAKE_SOURCE_DIR}/cmakes" )
SET( trEXT_LIBS_DIR "${CMAKE_SOURCE_DIR}/.extlibs" )
SET( trINCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include" )

# список исходников
file( GLOB trSRC "${CMAKE_SOURCE_DIR}/src/*.cpp" )

include( "${trCMAKES}/main.cmake" )
