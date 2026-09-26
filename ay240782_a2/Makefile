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

PROGRAMS = partA partB

all: $(PROGRAMS)

partA: partA.cpp InitShader.cpp InitShader.h
	$(CXX) $(CXXFLAGS) -o partA partA.cpp InitShader.cpp $(LIBS)

partB: partB.cpp InitShader.cpp InitShader.h
	$(CXX) $(CXXFLAGS) -o partB partB.cpp InitShader.cpp $(LIBS)

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean
