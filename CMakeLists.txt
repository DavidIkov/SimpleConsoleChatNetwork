cmake_minimum_required(VERSION 3.25)
project(chat LANGUAGES C CXX)

add_executable(${PROJECT_NAME}1
    main.cpp
    networking/socket_tcp.cpp
    networking/socket_tcp.hpp
)
add_executable(${PROJECT_NAME}2
    main2.cpp
    networking/socket_tcp.cpp
    networking/socket_tcp.hpp
)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set_property(TARGET ${PROJECT_NAME}1 ${PROJECT_NAME}2 PROPERTY CXX_STANDARD 17)
