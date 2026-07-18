#include "findBest.h"
#include "legalMoves.h"
#include "evaluate.h"
#include "board.h"
#include <limits.h>
#include <string.h>
#include <time.h>

#define MATE_SCORE 1000000

// Checking is our king in danger
bool isInCheck(const Board *board, int us) {
    int enemy = 1 - us;
    uint64_t king = board->pieces[us][5];
    return (king & getAttackedSquares(board, enemy)) != 0;
}

int negamax(Board *board, int depth, int alpha, int beta) {
    if (depth == 0) {
        int side = board->isWhiteTurn ? 1 : -1;
        return evaluate(board) * side;
    }

    char moves[MAX_MOVES][MOVE_STR_LEN];
    int moveCount = 0;
    legalMovesSearch(board, moves, &moveCount);

    if (moveCount == 0) {
        int us = board->isWhiteTurn ? 0 : 1;
        if (isInCheck(board, us)) {
            return -MATE_SCORE - depth;
        } else {
            return 0;
        }
    }

    int best = INT_MIN;
    for (int i = 0; i < moveCount; i++) {
        Board copy = *board;
        modifyBoard(&copy, moves[i]);

        int score = -negamax(&copy, depth - 1, -beta, -alpha);
        if (score > best) {
            best = score;
        }
        if (best > alpha) {
            alpha = best;
        }
        if (alpha >= beta) {
            break; // idiot move, dont check
        }
    }

    return best;
}

int findBest(Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int moveCount, int threads, int maxDepth, int timeBudgetMs) {
    if (moveCount <= 0) {
        return -1;
    }

    clock_t startTime = clock();
    int bestIndex = 0;

    for (int depth = 1; depth <= maxDepth; depth++) {
        int currentBestIndex = 0;
        int bestScore = INT_MIN;

        for (int i = 0; i < moveCount; i++) {
            Board copy = *board;
            modifyBoard(&copy, moves[i]);

            int score = -negamax(&copy, depth - 1, INT_MIN + 1, INT_MAX - 1);

            if (score > bestScore) {
                bestScore = score;
                currentBestIndex = i;
            }

            // Checking times
            if (timeBudgetMs > 0) {
                double elapsedMs = (double)(clock() - startTime) * 1000.0 / CLOCKS_PER_SEC;
                if (elapsedMs >= timeBudgetMs) {
                    return bestIndex; // Returning best move from previous depth
                }
            }
        }

        bestIndex = currentBestIndex; // Since this depth is done we are saving the result

        if (timeBudgetMs > 0) {
            double elapsedMs = (double)(clock() - startTime) * 1000.0 / CLOCKS_PER_SEC;
            if (elapsedMs >= timeBudgetMs) {
                break;
            }
        }
    }

    return bestIndex;
}