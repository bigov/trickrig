
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

SET( MY_FLAGS "-m64 -std=c++17 -fexceptions -Werror -Wpedantic \
  -Wextra -Woverloaded-virtual -Wctor-dtor-privacy -Wnon-virtual-dtor -Wall\
  -Winit-self -Wunreachable-code" )

## Пока работаем без установки программы
SET( SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE )

## Установка параметров для сборки в MinGW-W64 под MS Windows
if( (${MINGW}) AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows") )
  SET( CMAKE_CXX_FLAGS "${MY_FLAGS}" )
  #link_directories( "tp_libs/bin_mw64" )
  if(NOT ${CLANG})
    # --static отключен из-за отсутствия статической сборки libenet
    #SET( CMAKE_CXX_FLAGS "--static ${CMAKE_CXX_FLAGS} ${WIN_GUI}" )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WIN_GUI}" )
  endif()
  SET( trLIBS mingw32 png_static gdi32 )
  SET( BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE )
endif( (${MINGW}) AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows") )

## Установка параметров для сборки на Linux
if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  SET( CMAKE_CXX_FLAGS "${MY_FLAGS} -no-pie" )
  SET( trLIBS png )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

