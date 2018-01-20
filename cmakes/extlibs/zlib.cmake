
# библиотека zlib
SET( trINCLUDE_DIRS ${trINCLUDE_DIRS} "${trEXT_LIBS_DIR}/zlib" )
add_subdirectory( "${trEXT_LIBS_DIR}/zlib" "${SUFFIX}/zlib" EXCLUDE_FROM_ALL )

SET( ZLIB_INCLUDE_DIR "${trEXT_LIBS_DIR}/zlib" "${CMAKE_CURRENT_BINARY_DIR}/${SUFFIX}/zlib")
