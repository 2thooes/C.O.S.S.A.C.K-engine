#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "board.h"

void printBinary64(uint64_t bitboard);
void legalMovesSearch(const Board *board,char **moves);