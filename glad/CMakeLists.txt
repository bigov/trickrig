cmake_minimum_required( VERSION 3.0 )
project( libglad )

#target_include_directories( glad INTERFACE "../${CMAKE_SOURCE_DIR}" )

# On Linux platform glad require: -libdl
if   ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  SET( LIBS GL "dl" )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

ADD_LIBRARY( glad STATIC "glad.c" )
TARGET_LINK_LIBRARIES( glad ${LIBS} )

