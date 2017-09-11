cmake_minimum_required(VERSION 2.6)
project(lr_parser)

#install(TARGETS lr_parser RUNTIME DESTINATION bin)

SET(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall -Wextra -Wconversion -Wshadow")

add_subdirectory(src)

add_executable(lr_parser 
main.cpp
src/parser.cpp
)
