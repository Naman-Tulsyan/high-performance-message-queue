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
    src/broker.cpp
    src/producer.cpp
    src/consumer.cpp
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

add_executable(hmq_tcp_server_test tests/tcp_server_test.cpp)
target_link_libraries(hmq_tcp_server_test PRIVATE hmq_core)

add_executable(hmq_tcp_client_test tests/tcp_client_test.cpp)
target_link_libraries(hmq_tcp_client_test PRIVATE hmq_core)

add_executable(hmq_byte_order_test tests/byte_order_test.cpp)
target_link_libraries(hmq_byte_order_test PRIVATE hmq_core)

add_executable(hmq_broker_test tests/broker_test.cpp)
target_link_libraries(hmq_broker_test PRIVATE hmq_core)

add_executable(hmq_producer_consumer_test tests/producer_consumer_test.cpp)
target_link_libraries(hmq_producer_consumer_test PRIVATE hmq_core)

add_executable(hmq_client_api_test tests/client_api_test.cpp)
target_link_libraries(hmq_client_api_test PRIVATE hmq_core)

if(BUILD_TESTING)
    add_test(NAME hmq_smoke_test COMMAND hmq_smoke_test)
    add_test(NAME hmq_message_queue_test COMMAND hmq_message_queue_test)
    add_test(NAME hmq_message_log_test COMMAND hmq_message_log_test)
    add_test(NAME hmq_protocol_test COMMAND hmq_protocol_test)
    add_test(NAME hmq_tcp_server_test COMMAND hmq_tcp_server_test $<TARGET_FILE:hmq_server>)
    add_test(NAME hmq_tcp_client_test COMMAND hmq_tcp_client_test)
    add_test(NAME hmq_byte_order_test COMMAND hmq_byte_order_test)
    add_test(NAME hmq_broker_test COMMAND hmq_broker_test)
    add_test(NAME hmq_producer_consumer_test COMMAND hmq_producer_consumer_test $<TARGET_FILE:hmq_server>)
    add_test(NAME hmq_client_api_test COMMAND hmq_client_api_test)

    set_tests_properties(hmq_client_api_test PROPERTIES TIMEOUT 10)
    set_tests_properties(hmq_producer_consumer_test PROPERTIES TIMEOUT 10)
endif()
