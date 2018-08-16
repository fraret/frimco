cmake_minimum_required(VERSION 2.6)
project(tdr2)

add_executable(tdr2 main.cpp)

install(TARGETS tdr2 RUNTIME DESTINATION bin)
