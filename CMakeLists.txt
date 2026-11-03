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
    src/broker/broker.cpp
    src/broker/partition_store.cpp
    src/storage/message_log.cpp
    src/storage/message_queue.cpp
    src/network/tcp_server.cpp
    src/network/tcp_client.cpp
    src/protocol/protocol.cpp
    src/client/producer.cpp
    src/client/consumer.cpp
)

target_include_directories(hmq_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
)

target_compile_options(hmq_core PRIVATE
    -Wall -Wextra -Wpedantic
)

add_executable(hmq apps/hmq_main.cpp)
target_link_libraries(hmq PRIVATE hmq_core)
target_include_directories(hmq PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/internal)

add_executable(hmq_server apps/server_main.cpp)
target_link_libraries(hmq_server PRIVATE hmq_core)
target_include_directories(hmq_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/internal)

add_executable(hmq_client apps/client_main.cpp)
target_link_libraries(hmq_client PRIVATE hmq_core)
target_include_directories(hmq_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/internal)

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

add_executable(hmq_topic_partition_test tests/topic_partition_test.cpp)
target_link_libraries(hmq_topic_partition_test PRIVATE hmq_core)

add_executable(hmq_partition_paths_test tests/partition_paths_test.cpp)
target_link_libraries(hmq_partition_paths_test PRIVATE hmq_core)

add_executable(hmq_partition_store_test tests/partition_store_test.cpp)
target_link_libraries(hmq_partition_store_test PRIVATE hmq_core)

foreach(test_target IN ITEMS
    hmq_smoke_test
    hmq_message_queue_test
    hmq_message_log_test
    hmq_protocol_test
    hmq_tcp_server_test
    hmq_tcp_client_test
    hmq_byte_order_test
    hmq_broker_test
    hmq_producer_consumer_test
    hmq_client_api_test
    hmq_topic_partition_test
    hmq_partition_paths_test
    hmq_partition_store_test
)
    target_include_directories(${test_target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
    )
endforeach()

target_include_directories(hmq_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
)

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
    add_test(NAME hmq_topic_partition_test COMMAND hmq_topic_partition_test)
    add_test(NAME hmq_partition_paths_test COMMAND hmq_partition_paths_test)
    add_test(NAME hmq_partition_store_test COMMAND hmq_partition_store_test)

    set_tests_properties(hmq_client_api_test PROPERTIES TIMEOUT 10)
    set_tests_properties(hmq_producer_consumer_test PROPERTIES TIMEOUT 10)
endif()
