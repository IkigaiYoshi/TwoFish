cmake_minimum_required(VERSION 3.12)
project(Twofish)

set(CMAKE_CXX_STANDARD 17)

add_executable(Twofish main.cpp main.cpp workWithMessages.h Twofish.cpp Twofish.h)