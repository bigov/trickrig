
# библиотека glad
SET( trINCLUDE_DIRS ${trINCLUDE_DIRS} "${trEXT_LIBS_DIR}/glad" )
add_subdirectory( "${trEXT_LIBS_DIR}/glad" "${SUFFIX}/glad" EXCLUDE_FROM_ALL)
