#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#define MAX_MOVES 256
#define MOVE_STR_LEN 6

void coordsToMoveStr(int from, int to, char *str, bool is_promotion);
void whitePawns(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void blackPawns(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void KnightMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void bishopMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void rookMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void queenMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void kingMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void legalMovesSearch(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);