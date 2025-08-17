cmake_minimum_required(VERSION 3.22)
project(chat LANGUAGES C CXX)

add_library(networking 
    socket/socket_tcp.hpp
    socket/socket_tcp.cpp)
set_property(TARGET networking PROPERTY CXX_STANDARD 17)

add_executable(helloworld_example_client
    chat/synchronized_value.hpp
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
    chat/synchronized_value.hpp
    chat/events.hpp
    chat/events_handler.cpp
    chat/events_handler.hpp
    chat/helloworld_example/server.cpp
)
target_link_libraries(helloworld_example_server networking stdc++)
set_property(TARGET helloworld_example_server PROPERTY CXX_STANDARD 17)
set_property(TARGET helloworld_example_server PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(helloworld_example_server PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(chat_client
    chat/synchronized_value.hpp
    chat/events.hpp
    chat/shared.cpp
    chat/shared.hpp
    chat/events_handler.cpp
    chat/events_handler.hpp
    chat/client/connection.cpp
    chat/client/connection.hpp
    chat/client/user.cpp
    chat/client/user.hpp
    chat/client/main.cpp
)
target_link_libraries(chat_client networking stdc++)
set_property(TARGET chat_client PROPERTY CXX_STANDARD 17)
set_property(TARGET chat_client PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(chat_client PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(chat_server
    chat/synchronized_value.hpp
    chat/events.hpp
    chat/shared.cpp
    chat/shared.hpp
    chat/events_handler.cpp
    chat/events_handler.hpp
    chat/server/connections.cpp
    chat/server/connections.hpp
    chat/server/users.cpp
    chat/server/users.hpp
    chat/server/client/connection.cpp
    chat/server/client/connection.hpp
    chat/server/client/user.cpp
    chat/server/client/user.hpp
    chat/server/main.cpp
)
target_link_libraries(chat_server networking stdc++)
set_property(TARGET chat_server PROPERTY CXX_STANDARD 17)
set_property(TARGET chat_server PROPERTY EXPORT_COMPILE_COMMANDS 1)
target_include_directories(chat_server PRIVATE ${PROJECT_SOURCE_DIR})


