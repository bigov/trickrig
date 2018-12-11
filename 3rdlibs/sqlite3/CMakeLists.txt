#
# Append Sqlite3 library
#

cmake_minimum_required( VERSION 3.0 )
project( sqlite3 )
ADD_LIBRARY( sqlite3 STATIC "sqlite3-all.c" )
