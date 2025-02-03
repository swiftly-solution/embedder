cmake_minimum_required(VERSION 3.4)

project(SwiftlyEmbedder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "*.cpp")
file(GLOB EMBEDDER "../src/**/*.cpp")

include_directories("${CMAKE_SOURCE_DIR}/../libs/lua" "${CMAKE_SOURCE_DIR}/../libs/js")
add_executable(runner ${SOURCES} ${EMBEDDER})
target_link_libraries(runner ${CMAKE_SOURCE_DIR}/../libs/lua/liblua.a ${CMAKE_SOURCE_DIR}/../libs/js/libqjs.a)