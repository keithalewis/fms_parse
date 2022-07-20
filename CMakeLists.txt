cmake_minimum_required(VERSION 3.8)

project("fms_parse" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)

add_library(${PROJECT_NAME}_interface INTERFACE)
target_include_directories(${PROJECT_NAME}_interface INTERFACE ${PROJECT_SOURCE_DIR})

enable_testing ()
add_executable(fms_parse.t fms_parse.t.cpp)
add_test (NAME fms_parse.t COMMAND fms_parse.t)
