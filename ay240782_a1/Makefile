UNAME := $(shell uname -s)

ifeq ($(UNAME),Darwin)
    CXX      = clang++
    CXXFLAGS = -std=c++11 -Wall
    LIBS     = -framework OpenGL -framework GLUT
else
    CXX      = g++
    CXXFLAGS = -std=c++11 -Wall
    LIBS     = -lGL -lGLEW -lglut
endif

PROGRAMS = red_triangle blue_square

all: $(PROGRAMS)

red_triangle: red_triangle.cpp InitShader.cpp InitShader.h
	$(CXX) $(CXXFLAGS) -o red_triangle red_triangle.cpp InitShader.cpp $(LIBS)

blue_square: blue_square.cpp InitShader.cpp InitShader.h
	$(CXX) $(CXXFLAGS) -o blue_square blue_square.cpp InitShader.cpp $(LIBS)

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean
