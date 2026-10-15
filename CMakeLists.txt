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

add_library(hmq_core
    src/message_queue.cpp
)

target_include_directories(hmq_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(hmq_core PRIVATE
    -Wall -Wextra -Wpedantic
)

add_executable(hmq src/main.cpp)
target_link_libraries(hmq PRIVATE hmq_core)

add_executable(hmq_smoke_test tests/smoke_test.cpp)
target_link_libraries(hmq_smoke_test PRIVATE hmq_core)

add_executable(hmq_message_queue_test
    tests/message_queue_test.cpp
)
target_link_libraries(hmq_message_queue_test PRIVATE hmq_core)

if(BUILD_TESTING)
    add_test(NAME hmq_smoke_test COMMAND hmq_smoke_test)
    add_test(NAME hmq_message_queue_test COMMAND hmq_message_queue_test)
endif()
