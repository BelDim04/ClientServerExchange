CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

file(GLOB_RECURSE CORE *.cpp *.hpp)

add_library(Core ${CORE})

target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR})
