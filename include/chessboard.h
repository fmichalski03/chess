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
char gameDecider(Chessboard &board, char turn);
bool check(Chessboard& board, char turn, int king_pos[2]); // Check if the current player's king is in check
bool can_pawn_move(Chessboard &board, const int move[4]);  // Check if a pawn can legally move
bool can_knight_move(const Chessboard &board, const int move[4]); // Check if a knight can legally move
bool can_bishop_move(const Chessboard &board, const int move[4]); // Check if a bishop can legally move
bool can_rook_move(const Chessboard &board, const int move[4]);   // Check if a rook can legally move
bool can_queen_move(const Chessboard &board, const int move[4]);  // Check if a queen can legally move
bool can_king_move(const Chessboard &board, const int move[4]);   // Check if a king can legally move
bool stalemate(Chessboard &board, char turn, int king_pos[]);
bool checkmate(Chessboard &board, char turn, int king_pos[]);
void king_position(Chessboard &board, char turn, int king_pos[]);

#endif // CHESSBOARD_H
