# Utwórz biblioteki
add_library(chessboard chessboard.cpp)
add_library(interface interface.cpp)

# Dodaj katalog nagłówków do bibliotek
target_include_directories(chessboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Podłącz SFML do bibliotek (jeśli potrzebne)
target_link_libraries(chessboard sfml-system sfml-window sfml-graphics)
target_link_libraries(interface sfml-system sfml-window sfml-graphics)

# Pliki źródłowe główne
add_executable(server server.cpp)
add_executable(client client.cpp)

# Podłączenie bibliotek do aplikacji
target_link_libraries(server chessboard interface sfml-system sfml-window sfml-graphics)
target_link_libraries(client chessboard interface sfml-system sfml-window sfml-graphics)

