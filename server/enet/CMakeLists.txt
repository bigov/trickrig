#
# TrickRig udp server based in ENet lib.
#

set(PROJECT_AUTHOR "bigov")
set(PROJECT_AUTHOR_EMAIL "bigov@yandex.ru")

cmake_minimum_required( VERSION 3.2 )
project( server VERSION 0.0.1 )
#SET( BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE )
SET( CMAKE_CXX_FLAGS "-std=c++14 -D_FILE_OFFSET_BITS=64 -O2 -Wall\
 -Wundef -Wunused-parameter -Wno-ctor-dtor-privacy -Woverloaded-virtual" )

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

## Пока работаем без установки программы
SET( SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE )

# серверная часть
SET( SRV_SRC "server.cpp" )
# управление/контроль
SET( CTRL_SRC "ctrl.cpp" )

find_package( PkgConfig REQUIRED )
pkg_check_modules( ENET REQUIRED libenet )

add_executable( server ${SRV_SRC} )
target_link_libraries( server ${ENET_LIBRARIES} )

add_executable( ctrl ${CTRL_SRC} )
target_link_libraries( ctrl ${ENET_LIBRARIES} )
