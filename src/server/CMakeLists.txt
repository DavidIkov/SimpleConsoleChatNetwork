cmake_minimum_required(VERSION 3.31.3)
project(Server VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME} 
src/main.cpp 
src/ClientSlotClass/BasicClientSlot.cpp
src/ServerClass/BasicServer.cpp
src/ServerClass/EventsServer.cpp
src/ServerClass/ChatServer.cpp
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17)

set(RES_DIR "${PROJECT_SOURCE_DIR}/../res")

target_include_directories(${PROJECT_NAME} PRIVATE ${RES_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${RES_DIR}/asio-1.30.2/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${RES_DIR}/ConsoleManager)

add_subdirectory(${RES_DIR}/ConsoleManager ConsoleManagerBuild)
target_link_directories(${PROJECT_NAME} PRIVATE ConsoleManagerBuild)
target_link_libraries(${PROJECT_NAME} ConsoleManager)
