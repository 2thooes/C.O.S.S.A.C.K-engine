#include <stdio.h>
#include <stdlib.h>
#include "board.h"

#define MAX_MOVES 256
#define MOVE_STR_LEN 6

long getTimeMs();
bool isInCheck(const Board *board, int us);
int quiescence(Board * board,int alpha,int beta);
int negamax(Board *board, int depth, int alpha, int beta,int ply);
int findBest(Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int moveCount, int threads, int maxDepth, int timeBudgetMs);