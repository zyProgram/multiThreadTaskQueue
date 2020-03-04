cmake_minimum_required(VERSION 3.10)
project(_multiThreadTaskQueue)

set(CMAKE_CXX_STANDARD 11)

add_executable(_multiThreadTaskQueue main.cpp multiThreadTaskQueue.h)
target_link_libraries(_multiThreadTaskQueue pthread)