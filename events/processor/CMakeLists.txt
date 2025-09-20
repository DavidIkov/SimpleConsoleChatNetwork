cmake_minimum_required(VERSION 3.22)
project(events_processor LANGUAGES CXX)

add_library(events_processor STATIC processor.hpp processor.cpp)
set_property(TARGET events_processor PROPERTY CXX_STANDARD 17)
target_link_libraries(events_processor PRIVATE events_parser)
target_link_libraries(events_processor PUBLIC socket)
target_link_libraries(events_processor PUBLIC spdlog::spdlog)
target_include_directories(
  events_processor PRIVATE ${CMAKE_SOURCE_DIR}/third_party/spdlog/include)
