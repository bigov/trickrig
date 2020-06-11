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
add_subdirectory( "${CMAKE_SOURCE_DIR}/libs/wft" "wft" EXCLUDE_FROM_ALL )

## Список библиотек
SET( TR_LIBS ${TR_LIBS} ${GLFW_LIBRARIES} ${PNG_LIBRARIES} ${SQLITE_LIBRARIES}
   pthread stdc++fs glad wft)

# список исходников
file( GLOB MAIN_SRC "${CMAKE_SOURCE_DIR}/src/main/*.cpp" )
# расположение заголовков
include_directories( "${CMAKE_SOURCE_DIR}/src/main/include" )

add_executable( main ${MAIN_SRC} )
target_link_libraries( main ${TR_LIBS} )
