cmake_minimum_required( VERSION 3.0 )
project( glad )

# MS-Windows MinGW-W64 platform
if   ( (${MINGW}) AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows") )
  SET( LIBS opengl32 )
endif( (${MINGW}) AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows") )

# Linux platform
if   ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  SET( LIBS GL "dl" )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

ADD_LIBRARY( glad STATIC "glad.c" )
TARGET_LINK_LIBRARIES( glad ${LIBS} )
