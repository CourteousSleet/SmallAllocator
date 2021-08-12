cmake_minimum_required(VERSION 3.19)
project(SmartAllocator)

set(CMAKE_CXX_STANDARD 20)

add_executable(SmartAllocator SmartAllocator.cc)

install(TARGETS SmartAllocator DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin)