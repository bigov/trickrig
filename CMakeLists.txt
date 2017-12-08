#
# Сборка приложения - при вызове команды "cmake" НЕОБХОДИМО указать тип сборки:
# -DCMAKE_BUILD_TYPE=Release ../ -G "MinGW Makefiles"

# Потом (для всех)
#   cmake --build .

#
# Под Linux можно собрать так:
#
# cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Debug ../ && cmake --build . && ./app_dbg
# cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Release ../ && cmake --build . && ./app_rel
#
# https://cmake.org/Wiki/CMake_Useful_Variables
#

cmake_minimum_required( VERSION 3.0 )    # Проверка версии CMake.
project( trickrig )
SET( CMAKE_VERBOSE_MAKEFILE OFF )

# Для сборки релиза следует указать: "-D CMAKE_BUILD_TYPE=Release"
if( "_${CMAKE_BUILD_TYPE}" MATCHES "^_Rel" )
  OPTION( WITH_DEBUG_MODE "Build with debug mode" OFF )
  SET( CMAKE_BUILD_TYPE "Release")
  SET( SUFFIX "rel")
  SET( WIN_GUI "-mwindows -Wl,-subsystem,windows" )
  MESSAGE( WARNING "\n--- CMAKE_BUILD_TYPE: Release ---")
else() # Иначе собирается "Debug"
  OPTION( WITH_DEBUG_MODE "Build with debug mode" ON )
  SET( CMAKE_BUILD_TYPE "Debug")
  SET( SUFFIX "dbg")
  SET( WIN_GUI "" )
  MESSAGE( WARNING "\n--- CMAKE_BUILD_TYPE: Debug ---")
endif()

if("_$ENV{CC}" MATCHES "clang")
  SET( SUFFIX "${SUFFIX}_clang")
  SET(CLANG TRUE)
else()
  SET( SUFFIX "${SUFFIX}_gcc")
  SET(CLANG FALSE)
endif()

SET( EXEC_NAME "app${SUFFIX}" )

# библиотека LIBPNG
SET(PNG_SHARED OFF CACHE BOOL "Build shared lib" FORCE)
SET(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)
add_subdirectory( ".extlibs/libpng" "${SUFFIX}/libpng" EXCLUDE_FROM_ALL)

# библиотека freetype2
add_subdirectory( ".extlibs/freetype2" "${SUFFIX}/freetype" EXCLUDE_FROM_ALL)

# библиотека glcore33
add_subdirectory( ".extlibs/glcore33" "${SUFFIX}/glcore33" EXCLUDE_FROM_ALL)

# библиотека GLFW3
SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
SET(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
SET(GLFW_VULKAN_STATIC OFF CACHE BOOL "Use the Vulkan" FORCE)
add_subdirectory( ".extlibs/glfw" "${SUFFIX}/glfw" EXCLUDE_FROM_ALL)

# где искать заголовки
include_directories( "include" "lib"
 "${CMAKE_CURRENT_BINARY_DIR}/${SUFFIX}/libpng"
 ".extlibs/freetype2/include"
 ".extlibs/glfw/include"
 ".extlibs/glm"
 ".extlibs/libpng"
 ".extlibs/glcore33")

SET( MY_FLAGS "-m64 -std=c++14 -fexceptions -Werror -Wpedantic -Wextra\
 -Woverloaded-virtual -Wctor-dtor-privacy -Wnon-virtual-dtor -Wall\
 -Winit-self -Wunreachable-code" )

###
# Установка параметров для сборки в MinGW-W64 под MS Windows
#
if( (${MINGW}) AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows") )
  SET( CMAKE_CXX_FLAGS "${MY_FLAGS}" )
  if(NOT ${CLANG})
    SET( CMAKE_CXX_FLAGS "--static ${CMAKE_CXX_FLAGS} ${WIN_GUI}" )
    MESSAGE( WARNING "\n*** Static build is enabled. ***")
  endif()
  SET( LIBS mingw32 pthread glcore33 glfw freetype png_static gdi32 )
endif( (${MINGW}) AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows") )

###
# Установка параметров для сборки на Linux
#
if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
SET( CMAKE_CXX_FLAGS "${MY_FLAGS} -no-pie" )
SET( LIBS pthread glcore33 glfw freetype png )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

# список исходников
file( GLOB SRC "src/*.cpp" )

add_executable( ${EXEC_NAME} ${SRC} )
target_link_libraries( ${EXEC_NAME} ${LIBS} )
