#include "findBest.h"

int findBest(Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int moveCount){
    if (moveCount <= 0) {
        return -1; // ходов нет — мат или пат
    }
    return rand() % moveCount;
}