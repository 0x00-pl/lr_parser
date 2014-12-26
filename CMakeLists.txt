cmake_minimum_required(VERSION 2.6)
project(lr_parser)

add_executable(lr_parser main.cpp)

install(TARGETS lr_parser RUNTIME DESTINATION bin)
