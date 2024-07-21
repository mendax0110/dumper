cmake_minimum_required(VERSION 3.5)

project(dumper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCE_FILES
    src/Dumper.cpp
)

set(LIB_FILES
    examples/Hello.cpp
)

add_library(Hello SHARED ${LIB_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
