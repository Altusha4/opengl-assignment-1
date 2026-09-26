cmake_minimum_required(VERSION 3.16)
project(SunsetPostcard LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

add_executable(postcard main.cpp)
target_link_libraries(postcard PRIVATE glad glfw OpenGL::GL ${CMAKE_DL_LIBS})

if(MSVC)
    target_compile_options(postcard PRIVATE /W4)
else()
    target_compile_options(postcard PRIVATE -Wall -Wextra -Wpedantic)
endif()
