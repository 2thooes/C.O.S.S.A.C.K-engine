#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#define MAX_MOVES 256
#define MOVE_STR_LEN 6

void coordsToMoveStr(int from, int to, char *str, bool is_promotion);
void legalMovesSearch(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount);