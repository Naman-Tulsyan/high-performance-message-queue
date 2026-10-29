cmake_minimum_required(VERSION 3.20)

project(hmq
    VERSION 0.1.0
    DESCRIPTION "High Performance Message Queue"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(hmq_core
    src/message_queue.cpp
    src/message_log.cpp
    src/protocol.cpp
    src/tcp_server.cpp
    src/tcp_client.cpp
)

target_include_directories(hmq_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(hmq_core PRIVATE
    -Wall -Wextra -Wpedantic
)

add_executable(hmq src/main.cpp)
target_link_libraries(hmq PRIVATE hmq_core)

add_executable(hmq_server src/server_main.cpp)
target_link_libraries(hmq_server PRIVATE hmq_core)

add_executable(hmq_client src/client_main.cpp)
target_link_libraries(hmq_client PRIVATE hmq_core)

add_executable(hmq_smoke_test tests/smoke_test.cpp)
target_link_libraries(hmq_smoke_test PRIVATE hmq_core)

add_executable(hmq_message_queue_test tests/message_queue_test.cpp)
target_link_libraries(hmq_message_queue_test PRIVATE hmq_core)

add_executable(hmq_message_log_test tests/message_log_test.cpp)
target_link_libraries(hmq_message_log_test PRIVATE hmq_core)

add_executable(hmq_protocol_test tests/protocol_test.cpp)
target_link_libraries(hmq_protocol_test PRIVATE hmq_core)

if(BUILD_TESTING)
    add_test(NAME hmq_smoke_test COMMAND hmq_smoke_test)
    add_test(NAME hmq_message_queue_test COMMAND hmq_message_queue_test)
    add_test(NAME hmq_message_log_test COMMAND hmq_message_log_test)
    add_test(NAME hmq_protocol_test COMMAND hmq_protocol_test)
endif()
