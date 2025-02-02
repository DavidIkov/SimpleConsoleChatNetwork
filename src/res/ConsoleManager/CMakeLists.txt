cmake_minimum_required(VERSION 3.31.3)
project(ConsoleManager VERSION 0.1.0 LANGUAGES C CXX)

add_library(ConsoleManager ConsoleManager.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
