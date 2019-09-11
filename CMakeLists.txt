cmake_minimum_required(VERSION 2.6)
project(frimco)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCES
    src/*.h
    src/*.cpp
)


add_executable(frimco ${SOURCES})

install(TARGETS frimco RUNTIME DESTINATION bin)
