cmake_minimum_required( VERSION 3.0 )
project( glcore33 )
ADD_LIBRARY( glcore33 STATIC "gl_core33.c" )
TARGET_LINK_LIBRARIES( glcore33 opengl32 )
