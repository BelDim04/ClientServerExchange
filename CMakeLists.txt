CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(NTProExchange)

SET(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -g")
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
add_link_options(-Wall -Wextra -Wpedantic -Werror)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY archive)

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

file(GLOB_RECURSE SERVER server/server.cpp)
file(GLOB_RECURSE CLIENT client/*.cpp client/*.hpp)

add_subdirectory(common)
add_subdirectory(server/core)

add_executable(Server ${SERVER})
add_executable(Client ${CLIENT})

target_include_directories(Server PUBLIC .)
target_include_directories(Client PUBLIC .)

target_link_libraries(Server PRIVATE Threads::Threads ${Boost_LIBRARIES} Core Json Registry GodObject)
target_link_libraries(Client PRIVATE Threads::Threads ${Boost_LIBRARIES} Json Registry)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(tests tests/tests.cpp)
target_link_libraries(tests PRIVATE gtest_main gtest ${Boost_LIBRARIES} Core Json Registry GodObject)
target_include_directories(tests PUBLIC .)
add_test(NAME tests COMMAND ./${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests)
