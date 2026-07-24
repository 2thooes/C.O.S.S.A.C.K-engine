#include "timeManagement.h"

void calculateTimeBudget(Board *board,int wtime,int btime,int winc,int binc,int movestogo,int movetime,int depth,bool isInfinite,int *timeBudget,int *actualDepth){
    if(depth>0){
        *actualDepth = depth;
        *timeBudget = -1; // Without time limit
    }
    else if (movetime > 0) {
        *timeBudget = movetime;
        *actualDepth = 99; // Depth is restricted by time
    }
    else if (isInfinite) {
        *timeBudget = -1;
        *actualDepth = 99;
    }
    else{
        int myTime  = board->isWhiteTurn ? wtime : btime;
        int myInc   = board->isWhiteTurn ? winc  : binc;
        int movesLeft;
        if (myTime > 300000){movesLeft = 40;}
        else if (myTime > 120000){movesLeft = 30;}
        else if (myTime > 60000){movesLeft = 25;}
        else{
            if(movestogo > 0)
                movesLeft = movestogo;
            else
                movesLeft = 15;
            }
        // Simple rule: time/turn
        *timeBudget = (myTime / movesLeft) + (myInc / 2);
        if (*timeBudget > myTime / 2) {
            *timeBudget = myTime / 2;
        }
        *actualDepth = 99;
    }
}
