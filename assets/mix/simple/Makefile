#
# testing video
#

CPPFLAGS =-std=c++14
LDLIBS   =-lglfw -ldl -lpthread
SRC      =$(wildcard *.cpp) $(wildcard *.c)

# on MS-Win
#CPPFLAGS = $(CPPFLAGS) -mwindows -Wl,-subsystem,windows
#LDLIBS   = $(LDLIBS) -lmingw32 -lopengl32

all:
	c++ $(CPPFLAGS) $(SRC) $(LDLIBS) -o test
	./test
#	rm ./test

.PHONY: all main
