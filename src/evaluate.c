#include "evaluate.h"

int evaluate(Board *board){
    int evaluations[] = {100, 300, 300, 500, 900};

    int knightEval[64] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  0, 15, 15, 15, 15,  0,-30,
        -30,  0, 15, 15, 15, 15,  0,-30,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    int pawnEval[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        0, -5,-10,  0,  0,-10, -5,  0,
        0,-10,-10,-20,-20,-10,-10,  0,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    int rawEval = 0;

    // CalculatingMaterial
    for (int i = 0; i < sizeof(evaluations) / sizeof(evaluations[0]); i++){
        rawEval += (__builtin_popcountll(board->pieces[0][i]) 
                  - __builtin_popcountll(board->pieces[1][i])) * evaluations[i];
    }

    // PST of pawns
    uint64_t whitePawns = board->pieces[0][0];
    while (whitePawns) {
        int sq = __builtin_ctzll(whitePawns);
        rawEval += pawnEval[sq ^ 56];
        whitePawns &= whitePawns - 1;
    }

    uint64_t blackPawns = board->pieces[1][0];
    while (blackPawns) {
        int sq = __builtin_ctzll(blackPawns);
        rawEval -= pawnEval[sq]; // 180 degree for black
        blackPawns &= blackPawns - 1;
    }

    // PST коней
    uint64_t whiteKnights = board->pieces[0][1];
    while (whiteKnights) {
        int sq = __builtin_ctzll(whiteKnights);
        rawEval += knightEval[sq];
        whiteKnights &= whiteKnights - 1;
    }

    uint64_t blackKnights = board->pieces[1][1];
    while (blackKnights) {
        int sq = __builtin_ctzll(blackKnights);
        rawEval -= knightEval[sq ^ 56];
        blackKnights &= blackKnights - 1;
    }

    return rawEval;
}