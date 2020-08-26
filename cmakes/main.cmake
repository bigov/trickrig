#
# https://cmake.org/Wiki/CMake_Useful_Variables
#

pkg_check_modules( PNG REQUIRED libpng16 )
pkg_check_modules( GLFW REQUIRED glfw3 )
pkg_check_modules( SQLITE REQUIRED sqlite3 )
pkg_check_modules( GLM REQUIRED glm )

## Список библиотек
SET( TR_LIBS ${TR_LIBS} ${GLFW_LIBRARIES} ${PNG_LIBRARIES} ${SQLITE_LIBRARIES}
   pthread stdc++fs )

# On Linux platform glad require: -libdl
if   ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  SET( TR_LIBS ${TR_LIBS} GL "dl" )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

# список исходников
file( GLOB MAIN_SRC "${CMAKE_SOURCE_DIR}/src/main/*.cpp" "${CMAKE_SOURCE_DIR}/src/glad/glad.c" )

# расположение заголовков
include_directories( "${CMAKE_SOURCE_DIR}/src/main/include" )

add_executable( main ${MAIN_SRC} )
target_link_libraries( main ${TR_LIBS} )

add_custom_command(TARGET main POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/assets/ "assets")

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  SET( DLL_LIST libpng16-16.dll libsqlite3-0.dll glfw3.dll
      libwinpthread-1.dll zlib1.dll libstdc++-6.dll libgcc_s_seh-1.dll )
    find_path( DLL_DIR libstdc++-6.dll )
    foreach( DLL_LIB ${DLL_LIST} )
      add_custom_command( TARGET main POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy
                  "${DLL_DIR}/${DLL_LIB}" "${DLL_LIB}" )
    #  message( "${DIR0}/${LDLL}" )
  endforeach()
endif()
