cmake_minimum_required(VERSION 3.20)

project(hmq
    VERSION 0.1.0
    DESCRIPTION "High Performance Message Queue"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_executable(hmq
    src/main.cpp
)

target_compile_options(hmq PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(hmq_smoke_test
    tests/smoke_test.cpp
)

target_compile_options(hmq_smoke_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(BUILD_TESTING)
    add_test(NAME hmq_smoke_test COMMAND hmq_smoke_test)
endif()
