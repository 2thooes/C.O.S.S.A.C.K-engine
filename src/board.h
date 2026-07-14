// board.h
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // [0] - white, [1] - black
    // [0]-pawn, [1]-knight, [2]-bishop, [3]-rook, [4]-queen, [5]-king
    uint64_t pieces[2][6];

    uint64_t allWhite;
    uint64_t allBlack;

    uint64_t all;

    bool isWhiteTurn;
    uint8_t castleRights;
    int enPassant;
    int halfmoveClock;
} Board;

void printBoard(const Board *board);
void setStartpos(Board *board);
void parseFen(Board *board, const char *fen); // Unified snake_case
void modifyStartpos(Board *board, char *argument);

#endif // BOARD_H