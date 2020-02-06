
## Пока работаем без установки программы
SET( SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE )

# Для сборки релиза следует указать: "-D CMAKE_BUILD_TYPE=Release"
if( "_${CMAKE_BUILD_TYPE}" MATCHES "^_Rel" )
  OPTION( WITH_DEBUG_MODE "Build with debug mode" OFF )
  SET( CMAKE_BUILD_TYPE "Release")
  SET( SUFFIX "rel")
  MESSAGE( WARNING "\n--- CMAKE_BUILD_TYPE: Release ---")
else() # Иначе собирается "Debug"
  OPTION( WITH_DEBUG_MODE "Build with debug mode" ON )
  SET( CMAKE_BUILD_TYPE "Debug")
  SET( SUFFIX "dbg")
  MESSAGE( WARNING "\n--- CMAKE_BUILD_TYPE: Debug ---\n")
endif()

SET( CXX_FLAGS "-m64 -std=c++2a -fexceptions -Werror -Wpedantic -Wextra -Woverloaded-virtual \
  -Wctor-dtor-privacy -Wnon-virtual-dtor -Wall -Winit-self" ) ## -O3

SET( CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE )
SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie" )

if( "_$ENV{CC}" MATCHES "clang" )
  SET( CLANG TRUE )
  SET( SUFFIX "${SUFFIX}_clang" )
  SET( CXX_FLAGS "${CXX_FLAGS} -pthread" )
  SET( WIN_GUI "" ) # Clang матерится на -mwindows
  MESSAGE( WARNING "\n--- Compiling by Clang ---\n" )
else( "_$ENV{CC}" MATCHES "clang" )
  SET( CLANG FALSE )
  SET( SUFFIX "${SUFFIX}_gcc" )
  SET( CXX_FLAGS "${CXX_FLAGS} -Wunreachable-code -latomic -lpthread" )
endif( "_$ENV{CC}" MATCHES "clang" )

SET( EXEC_NAME "app${SUFFIX}" )

if( ${MINGW} )
  SET( trLIBS mingw32 gdi32 )
endif()

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  SET( CXX_FLAGS "${CXX_FLAGS} -mwindows -Wl,-subsystem,windows" )
endif()

SET( CMAKE_CXX_FLAGS "${CXX_FLAGS}" )

