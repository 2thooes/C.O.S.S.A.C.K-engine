#include <stdio.h>
#include "board.h"

void calculateTimeBudget(Board *board,int wtime,int btime,int winc,int binc,int movestogo,int movetime,int depth,bool isInfinite,int *timeBudget,int *actualDepth);