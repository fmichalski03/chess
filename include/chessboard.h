#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <vector>
#include <cstring>
#include <stdlib.h>


// Struktura reprezentująca pionek
struct Piece {
    char type;  // 'K', 'k', 'b', 'q', 'r', 'p', 'e' (empty)
    char color; // 'w' (white), 'b' (black), 'n' (none)

    Piece(char t = 'e', char c = 'n') : type(t), color(c) {}
};


using Chessboard = std::vector<std::vector<Piece>>;
Chessboard initializeBoard();
Chessboard initializeEndgameBoard();

void serializeChessboard(const Chessboard& board, int data[128]);
void deserializeChessboard(const int data[128], Chessboard& board);
bool can_move(Chessboard& board, int move[4], char turn);
bool check_mate(Chessboard &board, char turn);
bool stale_mate(Chessboard &board, char turn);

#endif // CHESSBOARD_H
