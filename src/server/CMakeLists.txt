cmake_minimum_required(VERSION 3.31.3)
project(Server VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME} 
src/main.cpp src/Client.cpp src/Server.cpp)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../res/asio-1.30.2/include)

include(CTest)
enable_testing()

