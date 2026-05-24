cmake_minimum_required(VERSION 3.20)
project(boost_asio_lab3 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Boost lookup ──────────────────────────────────────────────────────────────
# If Boost is not on PATH, set BOOST_ROOT here or pass -DBOOST_ROOT=... to cmake
# Example (uncomment and adjust):
# set(BOOST_ROOT "C:/local/boost_1_87_0")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# ── Helper macro ──────────────────────────────────────────────────────────────
function(add_task_target TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_include_directories(${TARGET_NAME} PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/task1
        ${CMAKE_SOURCE_DIR}/task2
        ${CMAKE_SOURCE_DIR}/task3
        ${CMAKE_SOURCE_DIR}/task4
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
        ${Boost_LIBRARIES}
        Threads::Threads
    )
    if(WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE ws2_32 mswsock)
    endif()
endfunction()

# ── Task 1 ────────────────────────────────────────────────────────────────────
add_task_target(task1_server task1/server.cpp)
add_task_target(task1_client task1/client.cpp)

# ── Task 2 ────────────────────────────────────────────────────────────────────
add_task_target(task2_server task2/server.cpp)
add_task_target(task2_client task2/client.cpp)

# ── Task 3 ────────────────────────────────────────────────────────────────────
add_task_target(task3_server task3/server.cpp)
add_task_target(task3_client task3/client.cpp)

# ── Task 4 ────────────────────────────────────────────────────────────────────
add_task_target(task4_server task4/server.cpp)
add_task_target(task4_client task4/client.cpp)
