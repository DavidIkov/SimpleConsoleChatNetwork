cmake_minimum_required(VERSION 3.22)
project(chat LANGUAGES CXX)

add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/sqlitecpp)
add_subdirectory(networking)
add_subdirectory(events)
add_subdirectory(helloworld_example)
add_subdirectory(chat)

