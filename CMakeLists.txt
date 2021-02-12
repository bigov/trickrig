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
SET( CMAKE_C_COMPILER clang CACHE STRING "clang compiler" FORCE )
SET( CMAKE_CXX_COMPILER clang++ CACHE STRING "clang++ compiler" FORCE )
SET( CMAKE_AR      llvm-ar )
SET( CMAKE_LINKER  llvm-ld )
SET( CMAKE_NM      llvm-nm )
SET( CMAKE_OBJDUMP llvm-objdump )
SET( CMAKE_RANLIB  llvm-ranlib )
SET( CLANG TRUE )

cmake_minimum_required( VERSION "3.15" )
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

project( TrickRig VERSION "0.0.6.0"
  DESCRIPTION "3D engine project"
  HOMEPAGE_URL "https://www.trickrig.net" )
SET( PROJECT_AUTHOR "Igor Balezin aka bigov" )
SET( PROJECT_AUTHOR_EMAIL "bigov@yandex.com" )

#  SET( CXX_FLAGS "${CXX_FLAGS} -pthread" )
#else()
#  SET( CLANG FALSE )
#  SET( CXX_FLAGS "${CXX_FLAGS} -Wunreachable-code -latomic -lpthread" )
#endif()

SET( SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE )
SET( CMAKE_VERBOSE_MAKEFILE OFF )

message(STATUS "Building for ${CMAKE_SYSTEM_NAME} target system")
message(STATUS "Building with ${CMAKE_CXX_COMPILER_ID} compiler")

configure_file( "${CMAKE_SOURCE_DIR}/cmakes/version_hpp.cmake" "${CMAKE_CURRENT_BINARY_DIR}/version.hpp" )

include_directories( SYSTEM "${CMAKE_SOURCE_DIR}/src" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )

# Для сборки релиза следует указать: "-D CMAKE_BUILD_TYPE=Release"
if( "_${CMAKE_BUILD_TYPE}" MATCHES "^_Rel" )
  OPTION( WITH_DEBUG_MODE "Build with debug mode" OFF )
  SET( CMAKE_BUILD_TYPE "Release")
else() # Иначе собирается "Debug"
  OPTION( WITH_DEBUG_MODE "Build with debug mode" ON )
  SET( CMAKE_BUILD_TYPE "Debug")
endif()

if( MSVC )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++2a --fexceptions" )
else( MSVC )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -fexceptions -Werror \
    -Wpedantic -Wextra -Woverloaded-virtual -Wctor-dtor-privacy \
    -Wnon-virtual-dtor -Wall -Winit-self " ) ## -O3")
endif( MSVC )

SET( CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE )

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
  -mwindows -Wl,-subsystem,windows " )
elseif()
  SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie" )
endif()

find_package( PkgConfig REQUIRED )

# main
include( "${CMAKE_SOURCE_DIR}/cmakes/main.cmake" )
