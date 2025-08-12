cmake_minimum_required(VERSION 3.22)
project(chat LANGUAGES C CXX)

add_library(networking 
    socket/socket_tcp.hpp
    socket/socket_tcp.cpp)
set_property(TARGET networking PROPERTY CXX_STANDARD 17)

add_executable(chat1
    main.cpp
)
target_link_libraries(chat1 networking stdc++)
set_property(TARGET chat1 PROPERTY CXX_STANDARD 17)
set_property(TARGET chat1 PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(chat1 PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(chat2
    main2.cpp
)
target_link_libraries(chat2 networking stdc++)
set_property(TARGET chat2 PROPERTY CXX_STANDARD 17)
set_property(TARGET chat2 PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(chat2 PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(helloworld_example_client
    chat/events.hpp
    chat/events_handler.cpp
    chat/events_handler.hpp
    chat/helloworld_example/client.cpp
)
target_link_libraries(helloworld_example_client networking stdc++)
set_property(TARGET helloworld_example_client PROPERTY CXX_STANDARD 17)
set_property(TARGET helloworld_example_client PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(helloworld_example_client PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(helloworld_example_server
    chat/events.hpp
    chat/events_handler.cpp
    chat/events_handler.hpp
    chat/helloworld_example/server.cpp
)
target_link_libraries(helloworld_example_server networking stdc++)
set_property(TARGET helloworld_example_server PROPERTY CXX_STANDARD 17)
set_property(TARGET helloworld_example_server PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(helloworld_example_server PRIVATE ${PROJECT_SOURCE_DIR})


