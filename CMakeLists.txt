cmake_minimum_required(VERSION 3.16)
project(ChessApp)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)


add_subdirectory(src)


set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_chessboard src/chessboard.cpp tests/test_chessboard.cpp)
target_link_libraries(test_chessboard GTest::GTest GTest::Main pthread)
