#
# https://cmake.org/Wiki/CMake_Useful_Variables
#

# установка параметров, необходимых для сборки проекта
include( "${trCMAKES}/params.cmake" )

#include( "${trCMAKES}/server.cmake" )

# подключение исходных файлов библиотек
include( "${trCMAKES}/extlibs/libpng.cmake" )
include( "${trCMAKES}/extlibs/glcore33.cmake" )
include( "${trCMAKES}/extlibs/sqlite3.cmake" )
include( "${trCMAKES}/extlibs/glfw.cmake" )
include( "${trCMAKES}/extlibs/glm.cmake" )

#include( "${trCMAKES}/extlibs/freetype2.cmake" )
#include_directories( SYSTEM "tp_libs" )
#link_directories( "tp_libs/bin_mw64" )

find_package( PkgConfig REQUIRED )
pkg_check_modules( FT2 REQUIRED freetype2 )
include_directories( SYSTEM ${FT2_INCLUDE_DIRS} )

# где искать заголовки
include_directories( ${trINCLUDE_DIRS} )

## Список библиотек
SET( trLIBS ${trLIBS} pthread stdc++fs glcore33 glfw sqlite3 )

add_executable( ${EXEC_NAME} ${trSRC} )
target_link_libraries( ${EXEC_NAME} ${FT2_LIBRARIES} ${trLIBS} )

