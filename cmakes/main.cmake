#
# https://cmake.org/Wiki/CMake_Useful_Variables
#

find_package( PkgConfig REQUIRED )
pkg_check_modules( PNG REQUIRED libpng16 )
pkg_check_modules( GLFW REQUIRED glfw3 )
pkg_check_modules( SQLITE REQUIRED sqlite3 )

pkg_check_modules( GLM REQUIRED glm )
include_directories( ${GLM_INCLUDE_DIRS} )

if( ${MINGW} )
  SET( trLIBS mingw32 gdi32 )
endif()

## Список библиотек
SET( trLIBS ${trLIBS} ${GLFW_LIBRARIES} ${PNG_LIBRARIES} ${SQLITE_LIBRARIES}
   pthread stdc++fs glad )

# список исходников
file( GLOB MAIN_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp" )

add_executable( main ${MAIN_SRC} )
target_link_libraries( main ${trLIBS} )
