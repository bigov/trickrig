
# библиотека LIBPNG
SET(PNG_BUILD_ZLIB ON CACHE BOOL "Собрать zlib самостоятельно" FORCE)
SET(PNG_DEBUG OFF CACHE BOOL "Build libpng with debug" FORCE)
SET(PNG_FRAMEWORK OFF CACHE BOOL "Build libpng with " FORCE)
SET(PNG_HARDWARE_OPTIMIZATIONS ON CACHE BOOL "Build libpng with HW-opt" FORCE)
SET(PNG_SHARED OFF CACHE BOOL "Build shared lib" FORCE)
SET(PNG_STATIC ON CACHE BOOL "Build libpng static" FORCE)
SET(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)

SET( trINCLUDE_DIRS ${trINCLUDE_DIRS} "${trEXT_LIBS_DIR}/libpng"
  "${CMAKE_CURRENT_BINARY_DIR}/${SUFFIX}/libpng" )

add_subdirectory( "${trEXT_LIBS_DIR}/libpng" "${SUFFIX}/libpng" EXCLUDE_FROM_ALL)
