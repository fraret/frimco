cmake_minimum_required(VERSION 2.6)
project(tdr2)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCES
    src/*.h
    src/*.cpp
)


add_executable(tdr2 ${SOURCES})

install(TARGETS tdr2 RUNTIME DESTINATION bin)
