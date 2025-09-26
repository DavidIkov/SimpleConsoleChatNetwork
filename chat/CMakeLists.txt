cmake_minimum_required(VERSION 3.22)
project(chat LANGUAGES CXX)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/events
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/events
          ${CMAKE_CURRENT_BINARY_DIR}/events
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/events
  COMMENT "copying events config")

add_custom_target(chat_events_config DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/events)

set(SQLiteCPP_INCLUDE ${CMAKE_SOURCE_DIR}/third_party/sqlitecpp/include)

add_executable(
  chat_client
  shared.hpp
  shared.cpp
  client/base.cpp
  client/base.hpp
  client/connection.cpp
  client/connection.hpp
  client/user.hpp
  client/user.cpp
  client/rooms.hpp
  client/rooms.cpp
  client/main.cpp)
target_link_libraries(chat_client events_processor SQLiteCpp)
set_property(TARGET chat_client PROPERTY CXX_STANDARD 17)
target_include_directories(chat_client PRIVATE ${SQLiteCPP_INCLUDE})
set_target_properties(chat_client PROPERTIES OUTPUT_NAME "client")
add_dependencies(chat_client chat_events_config)

add_executable(
  chat_server
  shared.hpp
  shared.cpp
  server/client/base.cpp
  server/client/base.hpp
  server/client/connection.cpp
  server/client/connection.hpp
  server/client/user.hpp
  server/client/user.cpp
  server/client/rooms.hpp
  server/client/rooms.cpp
  server/base.cpp
  server/base.hpp
  server/connections.cpp
  server/connections.hpp
  server/users.hpp
  server/users.cpp
  server/rooms.hpp
  server/rooms.cpp
  server/main.cpp)
target_link_libraries(chat_server events_processor SQLiteCpp)
set_property(TARGET chat_server PROPERTY CXX_STANDARD 17)
target_include_directories(chat_server PRIVATE ${SQLiteCPP_INCLUDE})
set_target_properties(chat_server PROPERTIES OUTPUT_NAME "server")
add_dependencies(chat_client chat_events_config)
