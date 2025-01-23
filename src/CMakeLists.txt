
add_library(chessboard chessboard.cpp)
add_library(interface interface.cpp)


target_include_directories(chessboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)


target_link_libraries(chessboard sfml-system sfml-window sfml-graphics)
target_link_libraries(interface sfml-system sfml-window sfml-graphics)


add_executable(server server.cpp)
add_executable(client client.cpp)


target_link_libraries(server chessboard interface sfml-system sfml-window sfml-graphics)
target_link_libraries(client chessboard interface sfml-system sfml-window sfml-graphics)

