#include "chessboard.h"


// Typ szachownicy
using Chessboard = std::vector<std::vector<Piece>>;

// Funkcja do inicjalizacji szachownicy
Chessboard initializeBoard() {
    Chessboard board(8, std::vector<Piece>(8, Piece()));

    // Ustawienie pionków
    for (int i = 0; i < 8; i++) {
        board[1][i] = Piece('p', 'w'); // Pionki białe
        board[6][i] = Piece('p', 'b'); // Pionki czarne
    }

    // Białe figury
    board[0][0] = board[0][7] = Piece('r', 'w');
    board[0][1] = board[0][6] = Piece('k', 'w');
    board[0][2] = board[0][5] = Piece('b', 'w');
    board[0][3] = Piece('q', 'w');
    board[0][4] = Piece('K', 'w');

    // Czarne figury
    board[7][0] = board[7][7] = Piece('r', 'b');
    board[7][1] = board[7][6] = Piece('k', 'b');
    board[7][2] = board[7][5] = Piece('b', 'b');
    board[7][3] = Piece('q', 'b');
    board[7][4] = Piece('K', 'b');

    return board;
}

// Funkcja serializacji szachownicy
// Funkcja serializująca szachownicę do tablicy int
void serializeChessboard(const Chessboard& board, int data[128]) {
    int index = 0;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece& piece = board[row][col];
            data[index++] = (int)piece.type;  // Serializacja typu figury
            data[index++] = (int)piece.color; // Serializacja koloru figury
        }
    }
}

// Funkcja deserializująca tablicę int do szachownicy
void deserializeChessboard(const int data[128], Chessboard& board) {
    int index = 0;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            board[row][col].type = (char)data[index++];  // Odtworzenie typu figury
            board[row][col].color = (char)data[index++]; // Odtworzenie koloru figury
        }
    }
}

bool can_pawn_move(const Chessboard& board, const int move[4]) {
    int x1 = move[0];
    int y1 = move[1];
    int x2 = move[2];
    int y2 = move[3];

    const Piece& pawn = board[y1][x1];

    // Sprawdzenie koloru pionka
    if (pawn.color == 'w') { // Biały pionek
        // Ruch o jedno pole do przodu
        if (y2 == y1 + 1 && x2 == x1 && board[y2][x2].type == 'e') {
            return true;
        }
        // Ruch o dwa pola do przodu na pierwszym ruchu
        if (y2 == y1 + 2 && x2 == x1 && y1 == 1 && board[y2][x2].type == 'e' && board[y1 + 1][x1].type == 'e') {
            return true;
        }
        // Bicie na skos
        if (y2 == y1 + 1 && (x2 == x1 + 1 || x2 == x1 - 1)) {
            const Piece& target = board[y2][x2];
            if (target.type != 'e' && target.color == 'b') { // Musi być przeciwnik
                return true;
            }
        }
    } else if (pawn.color == 'b') { // Czarny pionek
        // Ruch o jedno pole do przodu
        if (y2 == y1 - 1 && x2 == x1 && board[y2][x2].type == 'e') {
            return true;
        }
        // Ruch o dwa pola do przodu na pierwszym ruchu
        if (y2 == y1 - 2 && x2 == x1 && y1 == 6 && board[y2][x2].type == 'e' && board[y1 - 1][x1].type == 'e') {
            return true;
        }
        // Bicie na skos
        if (y2 == y1 - 1 && (x2 == x1 + 1 || x2 == x1 - 1)) {
            const Piece& target = board[y2][x2];
            if (target.type != 'e' && target.color == 'w') { // Musi być przeciwnik
                return true;
            }
        }
    }

    // Jeśli żaden warunek nie został spełniony, ruch jest nielegalny
    return false;
}

bool can_bishop_move(const Chessboard& board, const int move[4]) {
    int x1 = move[0]; // Starting x position
    int y1 = move[1]; // Starting y position
    int x2 = move[2]; // Target x position
    int y2 = move[3]; // Target y position

    const Piece& bishop = board[y1][x1];

    // Check if the move is diagonal
    if (abs(x2 - x1) != abs(y2 - y1)) {
        return false; // Not a valid bishop move
    }

    // Check for obstacles in the path
    int xDirection = (x2 - x1) > 0 ? 1 : -1; // Determine direction of x movement
    int yDirection = (y2 - y1) > 0 ? 1 : -1; // Determine direction of y movement

    int x = x1 + xDirection;
    int y = y1 + yDirection;

    while (x != x2 && y != y2) {
        if (board[y][x].type != 'e') { // If there's a piece in the way
            return false; // Move is blocked
        }
        x += xDirection;
        y += yDirection;
    }

    // Check if the target position has a piece of the same color
    const Piece& targetPiece = board[y2][x2];
    if (targetPiece.type != 'e' && targetPiece.color == bishop.color) {
        return false; // Cannot capture own piece
    }

    return true; // Valid move
}

bool can_knight_move(const Chessboard& board, const int move[4]) {
    int x1 = move[0]; // Starting x position
    int y1 = move[1]; // Starting y position
    int x2 = move[2]; // Target x position
    int y2 = move[3]; // Target y position

    const Piece& knight = board[y1][x1];

    // Check if the move is in an L-shape (2 squares in one direction and 1 square in the other)
    if ((abs(x2 - x1) == 2 && abs(y2 - y1) == 1) || (abs(x2 - x1) == 1 && abs(y2 - y1) == 2)) {
        // Check if the target position has a piece of the same color
        const Piece& targetPiece = board[y2][x2];
        if (targetPiece.type == 'e' || targetPiece.color != knight.color) {
            return true; // Valid move (either empty or capturing an opponent's piece)
        }
    }

    // If the move is not valid, return false
    return false;
}

bool can_move(Chessboard& board, int move[4], char turn) {

    bool state = false;
    int x1 = move[0];
    int y1 = move[1];
    int x2 = move[2];
    int y2 = move[3];

    if (board[y1][x1].color != turn){
        return false;
    }
    
    // Sprawdzenie, czy na pozycji początkowej znajduje się figura
    Piece& sourcePiece = board[y1][x1];
    if (sourcePiece.type == 'e') { // Brak figury (zakładamy, że '\0' oznacza puste pole)
        return false;
    }
    switch (sourcePiece.type)
    {
    case 'p':
        state = can_pawn_move(board, move);
        break;
    case 'b':
        state = can_bishop_move(board, move);
        break;
    case 'k':
        state = can_knight_move(board, move);
        break;
    default:
        break;
    }

    // Przeniesienie figury na nowe miejsce
    if (state){
        Piece& destinationPiece = board[y2][x2];
        destinationPiece = sourcePiece;   // Kopiowanie figury na nowe miejsce
        sourcePiece = Piece();            // Oczyszczenie pola początkowego (ustawienie na pusty Piece)
    }


    return state;
}

