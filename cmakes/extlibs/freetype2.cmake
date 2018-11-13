
# библиотека freetype2
SET(FT_DIR "tp_libs/freetype-2.9.1")
SET( trINCLUDE_DIRS ${trINCLUDE_DIRS} "${FT_DIR}/include" )
add_subdirectory( "${FT_DIR}" "${SUFFIX}/freetype" EXCLUDE_FROM_ALL)

#option(FT_WITH_ZLIB "Use system zlib instead of internal library." OFF)
#option(FT_WITH_BZIP2 "Support bzip2 compressed fonts." OFF)
#option(FT_WITH_PNG "Support PNG compressed OpenType embedded bitmaps." OFF)
#option(FT_WITH_HARFBUZZ "Improve auto-hinting of OpenType fonts." OFF)

#//Dependencies for the target
#freetype_LIB_DEPENDS:STATIC=general;D:/MSYS2/mingw64/lib/libz.dll.a;general;D:/MSYS2/mingw64/lib/libbz2.dll.a;general;D:/MSYS2/mingw64/lib/libpng.dll.a;general;D:/MSYS2/mingw64/lib/libz.dll.a;general;D:/MSYS2/mingw64/lib/libharfbuzz.dll.a;
  
# z.dll
# bz2.dll
# png.dll
# harfbuzz.dll

