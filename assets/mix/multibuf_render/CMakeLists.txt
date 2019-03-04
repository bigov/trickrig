#
# cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Debug ../ && cmake --build .
# cmake --clean . && cmake -DCMAKE_BUILD_TYPE=Release ../ && cmake --build .
#
cmake_minimum_required( VERSION 3.0 )

SET( PROJECT_AUTHOR "bigov" )
SET( PROJECT_AUTHOR_EMAIL "bigov@yandex.ru" )

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree built. Please create a build \
  directory outside of the source code and call cmake from there.")
endif()

project( "Multi-buffers" )
SET( EXEC_NAME "bin-go" )

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
endif()

### MS-Windows
#if(CMAKE_BUILD_TYPE STREQUAL "Release")
  # Не отображать окно терминала
#  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -Wl,-subsystem,windows")
#endif(CMAKE_BUILD_TYPE STREQUAL "Release")

# Контроль ошибок при сборке
SET( CMAKE_VERBOSE_MAKEFILE OFF )

## Сборка без установки программ
SET( SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -m64 -march=x86-64 \
  -Werror -Wall -Wextra -Wpedantic -Woverloaded-virtual -Wctor-dtor-privacy \
  -Wnon-virtual-dtor -Wconversion -Wsign-conversion -Winit-self \
  -Wunreachable-code -Weffc++ -Wold-style-cast -ldl" )

### On non-Windows platforms glad requires libdl,
### make sure to link with it (-ldl for gcc)!

# Настройка переменной пути к ${TP_LIBS}
#include( "../../deps/tplibs.cmake" )
#include_directories( SYSTEM "${TP_LIBS}" )

# Подключение библиотек
#SET( LIB_GLFW "${TP_LIBS}/GLFW/bin.mingw.64/glfw3.dll" )
#SET( LIB_GLAD "${TP_LIBS}/glad/bin.mingw.64/glad.dll" )

find_package( PkgConfig REQUIRED )
pkg_check_modules( GLFW REQUIRED glfw3 )

# Исходные файлы
file( GLOB SRC_CODE
    "main.cpp"
    "glfw_win.cpp"
    "gl_tools.cpp"
    "glad.c"
    )

add_executable( "${EXEC_NAME}" "${SRC_CODE}" )
target_link_libraries( "${EXEC_NAME}" "${GLFW_LIBRARIES}" )

