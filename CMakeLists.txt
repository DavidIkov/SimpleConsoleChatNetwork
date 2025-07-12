cmake_minimum_required(VERSION 3.25)
project(chat LANGUAGES C CXX)

add_library(networking 
    networking/socket_tcp.hpp
    networking/socket_tcp.cpp)
set_property(TARGET networking PROPERTY CXX_STANDARD 17)

add_executable(chat1
    main.cpp
)
target_link_libraries(chat1 networking)
set_property(TARGET chat1 PROPERTY CXX_STANDARD 17)
set_property(TARGET chat1 PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(chat1 PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(chat2
    main2.cpp
)
target_link_libraries(chat2 networking)
set_property(TARGET chat2 PROPERTY CXX_STANDARD 17)
set_property(TARGET chat2 PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(chat2 PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(chat
    chat/events.hpp
    chat/socket.cpp
    chat/socket.hpp
)
target_link_libraries(chat networking)
set_property(TARGET chat PROPERTY CXX_STANDARD 17)
set_property(TARGET chat PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(chat PRIVATE ${PROJECT_SOURCE_DIR})

