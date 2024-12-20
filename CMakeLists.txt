cmake_minimum_required(VERSION 3.16)
project(ChessApp)

# Ustawienia C++ (np. standard)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dodanie katalogów z nagłówkami
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Dodanie SFML
find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

# Podfoldery do zbudowania
add_subdirectory(src)

# Pliki zasobów
set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
