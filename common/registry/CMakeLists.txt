CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

add_subdirectory(${PROJECT_SOURCE_DIR}/common/god_object build_god)
add_subdirectory(${PROJECT_SOURCE_DIR}/common/json build_json)

add_library(Registry registry.cpp)

target_link_libraries(Registry PUBLIC GodObject Json)

target_include_directories(Registry PUBLIC ${PROJECT_SOURCE_DIR})
