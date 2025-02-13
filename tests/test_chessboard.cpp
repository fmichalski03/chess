#include "chessboard.h"
#include <gtest/gtest.h>

TEST(ChessboardTest, Initialization) {
    Chessboard board = initializeBoard();

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(board[1][i].type, 'p');
        EXPECT_EQ(board[1][i].color, 'w');
    }

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(board[6][i].type, 'p');
        EXPECT_EQ(board[6][i].color, 'b');
    }

    EXPECT_EQ(board[0][3].type, 'K');
    EXPECT_EQ(board[7][3].type, 'K');
    EXPECT_EQ(board[0][4].type, 'q');
    EXPECT_EQ(board[7][4].type, 'q');
}

TEST(ChessboardTest, PawnMove) {
    Chessboard board = initializeBoard();

    int move1[4] = {1, 1, 1, 3};
    EXPECT_TRUE(can_pawn_move(board, move1));

    int move2[4] = {1, 1, 1, 2};
    EXPECT_TRUE(can_pawn_move(board, move2));

    int move3[4] = {1, 1, 2, 2};
    EXPECT_FALSE(can_pawn_move(board, move3));
}

TEST(ChessboardTest, RookMove) {
    Chessboard board = initializeBoard();

    int move1[4] = {0, 0, 0, 5};
    EXPECT_FALSE(can_rook_move(board, move1));

    board[1][0] = Piece('e', ' ');

    EXPECT_TRUE(can_rook_move(board, move1));
}

TEST(ChessboardTest, KnightMove) {
    Chessboard board = initializeBoard();

    int move1[4] = {1, 0, 2, 2};
    EXPECT_TRUE(can_knight_move(board, move1));

    int move2[4] = {1, 0, 3, 3};
    EXPECT_FALSE(can_knight_move(board, move2));
}

TEST(ChessboardTest, QueenMove) {
    Chessboard board = initializeBoard();

    int move1[4] = {4, 0, 4, 4};
    EXPECT_FALSE(can_queen_move(board, move1));

    board[1][4] = Piece('e', ' ');

    EXPECT_TRUE(can_queen_move(board, move1));
}

TEST(ChessboardTest, CheckDetection) {
    Chessboard board = initializeEndgameBoard();

    int king_pos[2];
    king_position(board, 'w', king_pos);
    EXPECT_TRUE(check(board, 'w', king_pos));

    king_position(board, 'b', king_pos);
    EXPECT_FALSE(check(board, 'b', king_pos));
}

TEST(ChessboardTest, CheckmateDetection) {
    Chessboard board = initializeEndgameBoard();

    int king_pos[2];
    king_position(board, 'b', king_pos);
    EXPECT_FALSE(checkmate(board, 'b', king_pos));

    king_position(board, 'w', king_pos);
    EXPECT_TRUE(checkmate(board, 'w', king_pos));
}

TEST(ChessboardTest, StalemateDetection) {
    Chessboard board = initializeBoard();

    int king_pos[2];
    king_position(board, 'w', king_pos);
    EXPECT_FALSE(stalemate(board, 'w', king_pos));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}