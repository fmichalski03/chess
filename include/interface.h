// interface.h

#ifndef INTERFACE_H
#define INTERFACE_H

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cstring>

#include "chessboard.h"

// Funkcja do inicjalizacji okna
void window_init(sf::RenderWindow& window);
void load_pieces(std::map<std::string, sf::Texture>& pieceTextures, sf::Texture& chessboardTexture);
void window_display(sf::RenderWindow& window, const std::map<std::string, sf::Texture>& pieceTextures, const sf::Texture& chessboardTexture, Chessboard board, char side);
sf::Vector2i pixelToGrid(sf::Vector2i pixelPos); 

#endif // DISPLAY_H
