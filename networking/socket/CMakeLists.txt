cmake_minimum_required(VERSION 3.22)
project(socket LANGUAGES CXX)

add_library(socket STATIC
    tcp.hpp
    tcp.cpp
    common.hpp
)
target_link_libraries(socket PUBLIC spdlog::spdlog_header_only)
target_include_directories(socket PUBLIC ${CMAKE_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(socket INTERFACE ${CMAKE_SOURCE_DIR})
set_property(TARGET socket PROPERTY CXX_STANDARD 17)


