
# библиотека freetype2
SET( trINCLUDE_DIRS ${trINCLUDE_DIRS} "${trEXT_LIBS_DIR}/freetype2/include" )
add_subdirectory( "${trEXT_LIBS_DIR}/freetype2" "${SUFFIX}/freetype" EXCLUDE_FROM_ALL)
