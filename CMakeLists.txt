cmake_minimum_required(VERSION 3.20)

project(hmq
    VERSION 0.1.0
    DESCRIPTION "High Performance Message Queue"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(hmq apps/hmq_main.cpp)

add_executable(hmq_server apps/server_main.cpp)

add_executable(hmq_client apps/client_main.cpp)

foreach(app_target IN ITEMS
    hmq
    hmq_server
    hmq_client
)
    target_link_libraries(${app_target}
        PRIVATE hmq_core
    )
    target_include_directories(${app_target}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
    )
endforeach()

include(CTest)

if(BUILD_TESTING)
    function(hmq_add_test target source)

        add_executable(${target} ${source})

        target_link_libraries(${target} PRIVATE hmq_core)

        target_include_directories(${target}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
                ${CMAKE_CURRENT_SOURCE_DIR}
        )

        add_test(
            NAME ${target}
            COMMAND ${target}
        )

    endfunction()

    #  Unit Test
    hmq_add_test(
        hmq_byte_order_test
        tests/unit/byte_order_test.cpp
    )

    hmq_add_test(
        hmq_message_queue_test
        tests/unit/message_queue_test.cpp
    )

    hmq_add_test(
        hmq_partition_paths_test
        tests/unit/partition_paths_test.cpp
    )

    hmq_add_test(
        hmq_protocol_test
        tests/unit/protocol_test.cpp
    )

    hmq_add_test(
        hmq_topic_partition_test
        tests/unit/topic_partition_test.cpp
    )

    # Integration Tests

    hmq_add_test(
        hmq_broker_test
        tests/integration/broker_test.cpp
    )

    hmq_add_test(
        hmq_client_api_test
        tests/integration/client_api_test.cpp
    )

    hmq_add_test(
        hmq_message_log_test
        tests/integration/message_log_test.cpp
    )

    hmq_add_test(
        hmq_partition_store_test
        tests/integration/partition_store_test.cpp
    )

    hmq_add_test(
        hmq_smoke_test
        tests/integration/smoke_test.cpp
    )

    hmq_add_test(
        hmq_tcp_client_test
        tests/integration/tcp_client_test.cpp
    )

    # Tests requiring the server executable.
    add_executable(
        hmq_tcp_server_test
        tests/integration/tcp_server_test.cpp
    )

    add_executable(
        hmq_producer_consumer_test
        tests/integration/producer_consumer_test.cpp
    )

    foreach(test_target IN ITEMS
        hmq_tcp_server_test
        hmq_producer_consumer_test
    )
        target_link_libraries(${test_target}
            PRIVATE hmq_core
        )

        target_include_directories(${test_target}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
                ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()

    add_test(
        NAME hmq_tcp_server_test
        COMMAND hmq_tcp_server_test
            $<TARGET_FILE:hmq_server>
    )

    add_test(
        NAME hmq_producer_consumer_test
        COMMAND hmq_producer_consumer_test
            $<TARGET_FILE:hmq_server>
    )

    # Test Timeouts
    set_tests_properties(
        hmq_client_api_test
        hmq_producer_consumer_test
        PROPERTIES TIMEOUT 10
    )

endif()
