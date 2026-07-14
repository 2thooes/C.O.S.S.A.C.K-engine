#include "legalMoves.h"

void printBinary64(uint64_t bitboard) {
    // Проходим по всем 64 битам, начиная с самого старшего (63) к младшему (0)
    for (int i = 63; i >= 0; i--) {
        // Сдвигаем bitboard на i позиций вправо и проверяем младший бит
        uint64_t bit = (bitboard >> i) & 1ULL;
        printf("%llu", (unsigned long long)bit);
        
        // (Опционально) Для удобства чтения можно разделять байты пробелами:
        // if (i % 8 == 0 && i != 0) printf(" ");
    }
    printf("\n");
}

void legalMovesSearch(const Board *board,char **moves){
    uint64_t emptySquares = ~board->all;
    uint64_t PawnResult;
    if(board->isWhiteTurn){
        printf("generating legal moves for white ");
        //PAWNS
        printBinary64(board->pieces[0][0]);
        // Находим все поля перед БЕЛЫМИ пешками, которые СВОБОДНЫ
        uint64_t whitePawns = board->pieces[0][0];

        // Сдвигаем пешки на 1 клетку вверх и проверяем, свободны ли эти поля
        uint64_t whitePawnSemiResult = (whitePawns << 8) & emptySquares;
        printBinary64(whitePawnSemiResult);
    }
    else{
        printf("generating legal moves for black ");
        printBinary64(board->pieces[1][0]);
    } 
}