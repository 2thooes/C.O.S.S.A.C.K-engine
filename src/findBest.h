#include <stdio.h>
#include <stdlib.h>
#include "board.h"

#define MAX_MOVES 256
#define MOVE_STR_LEN 6

int findBest(Board * board,char moves[MAX_MOVES][MOVE_STR_LEN],int moveCount);