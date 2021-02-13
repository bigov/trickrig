#
# https://cmake.org/Wiki/CMake_Useful_Variables
#

#find_package( OpenGL REQUIRED )
#include_directories( ${OPENGL_INCLUDE_DIRS} )

pkg_check_modules( PNG REQUIRED libpng16 )
pkg_check_modules( SQLITE REQUIRED sqlite3 )

add_subdirectory( libs/glm )
#pkg_check_modules( GLM REQUIRED glm )
include_directories( ${GLM_INCLUDE_DIRS} )

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )

add_subdirectory( "libs/glfw" "${CMAKE_CURRENT_BINARY_DIR}/glfw/" EXCLUDE_FROM_ALL )
include_directories( ${GFW_INCLUDE_DIRS} )

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

## Список библиотек
##SET( TR_LIBS ${TR_LIBS} ${OPENGL_LIBRARIES} glfw ${PNG_LIBRARIES} ${SQLITE_LIBRARIES}
  SET( TR_LIBS ${TR_LIBS}                     glfw ${PNG_LIBRARIES} ${SQLITE_LIBRARIES}
   pthread stdc++fs )

# On Linux platform glad require: -libdl
if   ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  SET( TR_LIBS ${TR_LIBS} GL "dl" )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

# список исходников
file( GLOB MAIN_SRC "${CMAKE_SOURCE_DIR}/src/main/*.cpp" "${CMAKE_SOURCE_DIR}/libs/glad/glad.c" )

# расположение заголовков
include_directories( "${CMAKE_SOURCE_DIR}/src/main/include" )
include_directories( "libs" )

add_executable( TrickRig ${MAIN_SRC} )
target_link_libraries( TrickRig ${TR_LIBS} )

add_custom_command(TARGET TrickRig PRE_LINK
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/assets/ "assets")

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  SET( DLL_LIST libpng16-16.dll libsqlite3-0.dll libwinpthread-1.dll
                zlib1.dll libstdc++-6.dll libgcc_s_seh-1.dll
     )
    find_path( DLL_DIR libstdc++-6.dll )
    foreach( DLL_LIB ${DLL_LIST} )
      add_custom_command( TARGET TrickRig POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy
                  "${DLL_DIR}/${DLL_LIB}" "${DLL_LIB}" )
  endforeach()
endif()

if( MSVC )
  set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TrickRig )
endif() 
