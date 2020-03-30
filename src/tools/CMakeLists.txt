#
# Собка генератора данных вершин вокса
#
cmake_minimum_required( VERSION 3.15 )
project( faces_generator )
SET( PROJECT_AUTHOR "bigov" )
SET( PROJECT_AUTHOR_EMAIL "bigov@yandex.ru" )
SET( APP "faces_gen" )

SET( CMAKE_CXX_STANDARD "20")
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -std=c++2a -Werror -Wall -Wextra \
  -Wpedantic -Woverloaded-virtual -Winit-self -Wunreachable-code \
  -Wconversion -Wold-style-cast -Wctor-dtor-privacy -Wnon-virtual-dtor -Weffc++" )
SET( CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE )
SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie" )

include_directories( "${CMAKE_SOURCE_DIR}/../main" )
include_directories( "${CMAKE_SOURCE_DIR}/../../libs" )

configure_file( "${CMAKE_SOURCE_DIR}/../../cmakes/version_hpp.cmake" "${CMAKE_CURRENT_BINARY_DIR}/version.hpp" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )

add_executable( ${APP} "faces_gen.cpp" "vox.cpp" )
