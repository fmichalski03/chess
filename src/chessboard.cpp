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
    board[0][3] = Piece('K', 'w');
    board[0][4] = Piece('q', 'w');

    // Czarne figury
    board[7][0] = board[7][7] = Piece('r', 'b');
    board[7][1] = board[7][6] = Piece('k', 'b');
    board[7][2] = board[7][5] = Piece('b', 'b');
    board[7][3] = Piece('K', 'b');
    board[7][4] = Piece('q', 'b');

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

void king_position(Chessboard &board, char turn, int king_pos[2]){
    // Find the king's position
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].type == 'K' && board[i][j].color == turn)
            {
                king_pos[0] = i;
                king_pos[1] = j;
                return;
            }
        }
    }
            
}

bool check(Chessboard &board, char turn, int king_pos[2]) {
    // Iterate through the board to find if any opponent's piece can attack the king
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board[row][col].type != 'e' && board[row][col].color != turn) {
                // Create a move array representing the potential attack on the king
                int move[4] = {col, row, king_pos[1], king_pos[0]}; // (x1, y1, x2, y2)

                // Check if the piece can move to the king's position
                bool canAttack = false;
                switch (board[row][col].type) {
                    case 'p': canAttack = can_pawn_move(board, move); break;
                    case 'k': canAttack = can_knight_move(board, move); break;
                    case 'b': canAttack = can_bishop_move(board, move); break;
                    case 'r': canAttack = can_rook_move(board, move); break;
                    case 'q': canAttack = can_queen_move(board, move); break;
                    case 'K': canAttack = can_king_move(board, move); break;
                    default: break;
                }

                // If any piece can attack the king, return true (king is in check)
                if (canAttack) {
                    return true;
                }
            }
        }
    }

    // If no piece can attack the king, return false (king is not in check)
    return false;
}

bool check_mate(Chessboard &board, char turn) {
    int king_pos[2];
    king_position(board, turn, king_pos);

    // Check if the king is in check
    if (!check(board, turn, king_pos)) {
        return false; // Not in check, so not checkmate
    }

    // Iterate through all pieces to see if any can block or capture the attacking piece
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j].color == turn) {
                // Try all possible moves for this piece
                for (int x = 0; x < 8; x++) {
                    for (int y = 0; y < 8; y++) {
                        int move[4] = {j, i, y, x}; // {startX, startY, targetX, targetY}
                        if (can_move(board, move, turn)) {
                            // Simulate the move
                            Chessboard tempBoard = board;
                            Piece destinationPiece = tempBoard[y][x];
                            tempBoard[y][x] = tempBoard[i][j];
                            tempBoard[i][j] = Piece();

                            // Check if the king is still in check after the move
                            int tempKingPos[2];
                            king_position(tempBoard, turn, tempKingPos);
                            if (!check(tempBoard, turn, tempKingPos)) {
                                return false; // Found a move that gets the king out of check
                            }
                        }
                    }
                }
            }
        }
    }

    // If no moves can get the king out of check, it's checkmate
    return true;
}

bool stale_mate(Chessboard &board, char turn) {
    int king_pos[2];
    king_position(board, turn, king_pos);

    // Check if the king is in check
    if (check(board, turn, king_pos)) {
        return false; // If the king is in check, it's not a stalemate
    }
    Chessboard tempBoard = board;

    // Iterate through all pieces to see if any can make a legal move
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j].color == turn) {
                // Try all possible moves for this piece
                for (int x = 0; x < 8; x++) {
                    for (int y = 0; y < 8; y++) {
                        int move[4] = {j, i, y, x}; // {startX, startY, targetX, targetY}
                        if (can_move(tempBoard, move, turn)) {
                            // If any legal move is found, it's not a stalemate
                            return false;
                        }
                    }
                }
            }
        }
    }

    // If no legal moves are found and the king is not in check, it's a stalemate
    return true;
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
        // Check if the pawn has reached the last row
        if ((board[y2][x2].type == 'p')&&((board[y2][x2].color == 'w' && y2 == 7) || (board[y2][x2].color == 'b' && y2 == 0))) {
            // Promote the pawn to a queen
            board[y2][x2].type='q';
        }
        if(check(board, turn, king_pos)){
            board[y2][x2] = destinationPiece;
            board[y1][x1] = sourcePiece;
            return false;
        }
    }

    return state;
}
