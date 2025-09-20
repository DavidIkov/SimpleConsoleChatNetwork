cmake_minimum_required(VERSION 3.22)
project(events_parser LANGUAGES CXX)

if(NOT DEFINED ANTLR4_INCLUDE_DIR)
  message(FATAL_ERROR "ANTLR4_INCLUDE_DIR is not defined")
endif()
if(NOT DEFINED ANTLR4_SHARED_LIB)
  message(FATAL_ERROR "ANTLR4_SHARED_LIB is not defined")
endif()
if(NOT DEFINED ANTLR4_BINARY)
  message(FATAL_ERROR "ANTLR4_BINARY is not defined")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser)

set(PARSER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsBaseListener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsBaseListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsLexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsListener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser/eventsParser.cpp)

add_custom_command(
  OUTPUT ${PARSER_SOURCES}
  COMMAND ${ANTLR4_BINARY} -Dlanguage=Cpp ${CMAKE_CURRENT_SOURCE_DIR}/events.g4
          -o ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_parser
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/events.g4
  COMMENT "generating parser for events")

add_library(antlr4_parser STATIC ${PARSER_SOURCES})
set_property(TARGET antlr4_parser PROPERTY CXX_STANDARD 17)
target_include_directories(antlr4_parser PRIVATE ${ANTLR4_INCLUDE_DIR})
target_link_libraries(antlr4_parser ${ANTLR4_SHARED_LIB})

add_library(events_parser STATIC data_types.hpp parse.hpp parse.cpp)
set_property(TARGET events_parser PROPERTY CXX_STANDARD 17)
target_include_directories(events_parser PRIVATE ${ANTLR4_INCLUDE_DIR})
target_link_libraries(events_parser PRIVATE antlr4_parser
                                            spdlog::spdlog)
target_include_directories(
  events_parser PRIVATE ${CMAKE_SOURCE_DIR}/third_party/spdlog/include)
