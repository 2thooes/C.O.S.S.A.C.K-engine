#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#define MAX_MOVES 256
#define MOVE_STR_LEN 6

typedef struct {
    char bestMoves[MAX_MOVES/2][MOVE_STR_LEN]; // promotion
    char highMoves[MAX_MOVES/2][MOVE_STR_LEN]; //attacks
    char mediumMoves[MAX_MOVES/2][MOVE_STR_LEN]; //regular
    char lowMoves[MAX_MOVES/2][MOVE_STR_LEN]; //pawn 1 
    int bestCount;
    int highCount;
    int mediumCount;
    int lowCount;
} MovesStr;

void coordsToMoveStr(int from, int to, char *str, bool is_promotion);
bool isKingSafe(const Board * board, int from, int to, bool is_promotion);

void whitePawns(const Board *board, MovesStr* moves);
void blackPawns(const Board *board, MovesStr* moves);
void KnightMoves(const Board *board, MovesStr* moves);
void bishopMoves(const Board *board, MovesStr* moves);
void rookMoves(const Board *board, MovesStr* moves);
void queenMoves(const Board *board, MovesStr* moves);
void kingMoves(const Board *board, MovesStr* moves);

void attacksSearch(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);
void legalMovesSearch(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);