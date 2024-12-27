#include "chessboard.h"
#include <stdio.h>

// Typ szachownicy
using Chessboard = std::vector<std::vector<Piece>>;

bool check(Chessboard& board, char turn, int king_pos[2]);
bool can_pawn_move(Chessboard &board, const int move[4]);
bool can_knight_move(const Chessboard &board, const int move[4]);
bool can_bishop_move(const Chessboard &board, const int move[4]);
bool can_rook_move(const Chessboard &board, const int move[4]);
bool can_queen_move(const Chessboard &board, const int move[4]);
bool can_king_move(const Chessboard &board, const int move[4]);

// Funkcja do inicjalizacji szachownicy
Chessboard initializeBoard()
{
    Chessboard board(8, std::vector<Piece>(8, Piece()));

    // Ustawienie pionków
    for (int i = 0; i < 8; i++)
    {
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
void serializeChessboard(const Chessboard &board, int data[128])
{
    int index = 0;
    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            const Piece &piece = board[row][col];
            data[index++] = (int)piece.type;  // Serializacja typu figury
            data[index++] = (int)piece.color; // Serializacja koloru figury
        }
    }
}

// Funkcja deserializująca tablicę int do szachownicy
void deserializeChessboard(const int data[128], Chessboard &board)
{
    int index = 0;
    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            board[row][col].type = (char)data[index++];  // Odtworzenie typu figury
            board[row][col].color = (char)data[index++]; // Odtworzenie koloru figury
        }
    }
}

bool can_pawn_move(Chessboard &board, const int move[4]) {

    bool state = false;
    int startX = move[0];
    int startY = move[1];
    int targetX = move[2];
    int targetY = move[3];

    const Piece &pawn = board[startY][startX];
    char pawnColor = pawn.color;
    int direction = (pawnColor == 'w') ? 1 : -1; // 1 for white, -1 for black
    int startRow = (pawnColor == 'w') ? 1 : 6; // Starting row for pawns

    // Check for moving forward
    if (targetY == startY + direction && targetX == startX) {
        if (board[targetY][targetX].type == 'e') {
            state = true; // Move one square forward
        }
    }

    // Check for initial double move
    if (startY == startRow && targetY == startY + 2 * direction && targetX == startX) {
        if (board[startY + direction][startX].type == 'e' && board[targetY][targetX].type == 'e') {
            state = true; // Move two squares forward
        }
    }

    // Check for capturing
    if (targetY == startY + direction && (targetX == startX + 1 || targetX == startX - 1)) {
        const Piece &target = board[targetY][targetX];
        if (target.type != 'e' && target.color != pawnColor) {
            state = true; // Capture opponent's piece
        }
    }

    // If no conditions are met, the move is illegal
    return state;
}

bool can_bishop_move(const Chessboard &board, const int move[4])
{
    int x1 = move[0]; // Starting x position
    int y1 = move[1]; // Starting y position
    int x2 = move[2]; // Target x position
    int y2 = move[3]; // Target y position

    const Piece &bishop = board[y1][x1];

    // Check if the move is diagonal
    if (abs(x2 - x1) != abs(y2 - y1))
    {
        return false; // Not a valid bishop move
    }

    // Check for obstacles in the path
    int xDirection = (x2 - x1) > 0 ? 1 : -1; // Determine direction of x movement
    int yDirection = (y2 - y1) > 0 ? 1 : -1; // Determine direction of y movement

    int x = x1 + xDirection;
    int y = y1 + yDirection;

    while (x != x2 && y != y2)
    {
        if (board[y][x].type != 'e')
        {                 // If there's a piece in the way
            return false; // Move is blocked
        }
        x += xDirection;
        y += yDirection;
    }

    // Check if the target position has a piece of the same color
    const Piece &targetPiece = board[y2][x2];
    if (targetPiece.type != 'e' && targetPiece.color == bishop.color)
    {
        return false; // Cannot capture own piece
    }

    return true; // Valid move
}

bool can_knight_move(const Chessboard &board, const int move[4])
{
    int x1 = move[0]; // Starting x position
    int y1 = move[1]; // Starting y position
    int x2 = move[2]; // Target x position
    int y2 = move[3]; // Target y position

    const Piece &knight = board[y1][x1];

    // Check if the move is in an L-shape (2 squares in one direction and 1 square in the other)
    if ((abs(x2 - x1) == 2 && abs(y2 - y1) == 1) || (abs(x2 - x1) == 1 && abs(y2 - y1) == 2))
    {
        // Check if the target position has a piece of the same color
        const Piece &targetPiece = board[y2][x2];
        if (targetPiece.type == 'e' || targetPiece.color != knight.color)
        {
            return true; // Valid move (either empty or capturing an opponent's piece)
        }
    }

    // If the move is not valid, return false
    return false;
}

bool can_rook_move(const Chessboard &board, const int move[4])
{
    int x1 = move[0]; // Starting x position
    int y1 = move[1]; // Starting y position
    int x2 = move[2]; // Target x position
    int y2 = move[3]; // Target y position

    const Piece &rook = board[y1][x1];

    // Check if the move is horizontal or vertical
    if (x1 != x2 && y1 != y2)
    {
        return false; // Not a valid rook move
    }

    // Check for obstacles in the path
    int xDirection = (x2 - x1) == 0 ? 0 : (x2 - x1) > 0 ? 1
                                                        : -1; // Determine direction of x movement
    int yDirection = (y2 - y1) == 0 ? 0 : (y2 - y1) > 0 ? 1
                                                        : -1; // Determine direction of y movement

    int x = x1 + xDirection;
    int y = y1 + yDirection;

    while (x != x2 || y != y2)
    {
        if (board[y][x].type != 'e')
        {                 // If there's a piece in the way
            return false; // Move is blocked
        }
        x += xDirection;
        y += yDirection;
    }

    // Check if the target position has a piece of the same color
    const Piece &targetPiece = board[y2][x2];
    if (targetPiece.type != 'e' && targetPiece.color == rook.color)
    {
        return false; // Cannot capture own piece
    }

    return true; // Valid move
}

bool can_king_move(const Chessboard &board, const int move[4])
{
    int x1 = move[0]; // Starting x position
    int y1 = move[1]; // Starting y position
    int x2 = move[2]; // Target x position
    int y2 = move[3]; // Target y position

    const Piece &king = board[y1][x1];

    // Check if the move is one square in any direction
    if (abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1)
    {
        // Check if the target position has a piece of the same color
        const Piece &targetPiece = board[y2][x2];
        if (targetPiece.type != 'e' && targetPiece.color == king.color)
        {
            return false; // Cannot capture own piece
        }
        return true; // Valid move
    }

    return false; // Not a valid king move
}

bool can_queen_move(const Chessboard &board, const int move[4])
{
    // A queen can move like both a rook and a bishop
    if (can_rook_move(board, move) || can_bishop_move(board, move))
    {
        return true;
    }
    return false;
}

bool check(Chessboard &board, char turn, int king_pos[2]){
    // Check if the king is in check
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].type != 'e')
            {
                if (board[i][j].color == turn){
                    continue;
                }
                int move[4];
                move[0] = i;
                move[1] = j;
                move[2] = king_pos[1];
                move[3] = king_pos[0];
                Piece &sourcePiece = board[i][j];
                bool state = false;
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
                    case 'r':
                        state = can_rook_move(board, move);
                        break;
                    case 'q':
                        state = can_queen_move(board, move);
                        break;
                    case 'K':
                        state = can_king_move(board, move);
                        break;
                    default:
                        break;
                    }
                    if (state)
                    {
                        printf("%d %d %d %d", move[0], move[1], move[3], move[2]);
                        return true;
                    }
            }
        }

    }
    return false;
}

void king_position(Chessboard &board, char turn, int king_pos[2]){
    // Find the king's position
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].type == 'K' && board[i][j].color == turn)
            {
                king_pos[0] = j;
                king_pos[1] = i;
                return;
            }
        }
    }
            
}

bool can_move(Chessboard &board, int move[4], char turn)
{

    bool state = false;
    int x1 = move[0];
    int y1 = move[1];
    int x2 = move[2];
    int y2 = move[3];

    if (board[y1][x1].color != turn)
    {
        return false;
    }

    // Sprawdzenie, czy na pozycji początkowej znajduje się figura
    Piece sourcePiece = board[y1][x1];
    if (sourcePiece.type == 'e')
    { // Brak figury (zakładamy, że '\0' oznacza puste pole)
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
    case 'r':
        state = can_rook_move(board, move);
        break;
    case 'q':
        state = can_queen_move(board, move);
        break;
    case 'K':
        state = can_king_move(board, move);
        break;
    default:
        break;
    }

    // Przeniesienie figury na nowe miejsce
    if (state)
    {
        Piece destinationPiece = board[y2][x2];
        board[y2][x2] = sourcePiece; // Kopiowanie figury na nowe miejsce
        board[y1][x1] = Piece();          // Oczyszczenie pola początkowego (ustawienie na pusty Piece)
        int king_pos[2] = {0};
        king_position(board, turn, king_pos);
        if(check(board, turn, king_pos)){
            board[y2][x2] = destinationPiece;
            board[y1][x1] = sourcePiece;
            return false;
        }
    }

    return state;
}
