#
# https://cmake.org/Wiki/CMake_Useful_Variables
#

pkg_check_modules( PNG REQUIRED libpng16 )

pkg_check_modules( GLFW REQUIRED glfw3 )

pkg_check_modules( SQLITE REQUIRED sqlite3 )

pkg_check_modules( GLM REQUIRED glm )
include_directories( ${GLM_INCLUDE_DIRS} )

# glad             [CMakefile для glad          ]  [бинарник]
add_subdirectory( "${CMAKE_SOURCE_DIR}/libs/glad" "glad" EXCLUDE_FROM_ALL )

## Список библиотек
SET( TR_LIBS ${TR_LIBS} ${GLFW_LIBRARIES} ${PNG_LIBRARIES} ${SQLITE_LIBRARIES}
   pthread stdc++fs glad)

# список исходников
file( GLOB MAIN_SRC "${CMAKE_SOURCE_DIR}/src/main/*.cpp" )

# расположение заголовков
include_directories( "${CMAKE_SOURCE_DIR}/src/main/include" )

add_executable( main ${MAIN_SRC} )
target_link_libraries( main ${TR_LIBS} )

add_custom_command(TARGET main POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/assets/ "assets")

add_custom_command(TARGET main POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${LIBS_DIR}/libstdc++-6.dll libstdc++-6.dll)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  SET( SYS_LIBS_LIST libpng16-16.dll libsqlite3-0.dll glfw3.dll
      libwinpthread-1.dll zlib1.dll libstdc++-6.dll libgcc_s_seh-1.dll )
  find_path(SYS_LIBS_DIR libstdc++-6.dll)
  foreach(SYS_LIB ${SYS_LIBS_LIST} )
    add_custom_command( TARGET main POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${SYS_LIBS_DIR}/${SYS_LIB} ${SYS_LIB} )
  endforeach()
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
