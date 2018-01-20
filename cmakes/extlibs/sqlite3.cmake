
# библиотека sqlite3
SET( trINCLUDE_DIRS ${trINCLUDE_DIRS} "${trEXT_LIBS_DIR}/sqlite3" )
add_subdirectory( "${trEXT_LIBS_DIR}/sqlite3" "${SUFFIX}/sqlite3" EXCLUDE_FROM_ALL)
