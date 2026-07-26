#include "findBest.h"
#include "legalMoves.h"
#include "evaluate.h"
#include "board.h"
#include <limits.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define MATE_SCORE 1000000

#define MAX_DEPTH 64
char pvTable[MAX_DEPTH][MAX_DEPTH][6]; // [глубина_узла][индекс_хода_в_линии]
int pvLength[MAX_DEPTH];

long nodeCount = 0;

long getTimeMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000L + tv.tv_usec / 1000L;
}

// Checking is our king in danger
bool isInCheck(const Board *board, int us) {
    int enemy = 1 - us;
    uint64_t king = board->pieces[us][5];
    return (king & getAttackedSquares(board, enemy)) != 0;
}

int negamax(Board *board, int depth, int alpha, int beta, int ply) {
    nodeCount++;
    if (depth == 0) {
        pvLength[ply] = 0;  // на листе PV пустой
        int side = board->isWhiteTurn ? 1 : -1;
        return evaluate(board) * side;
    }

    char moves[MAX_MOVES][MOVE_STR_LEN];
    int moveCount = 0;
    legalMovesSearch(board, moves, &moveCount);

    if (moveCount == 0) {
        pvLength[ply] = 0;
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

        int score = -negamax(&copy, depth - 1, -beta, -alpha, ply + 1);

        if (score > best) {
            best = score;
            strcpy(pvTable[ply][0], moves[i]);
            memcpy(pvTable[ply][1], pvTable[ply + 1][0], pvLength[ply + 1] * 6);
            pvLength[ply] = pvLength[ply + 1] + 1;
        }

        if (best > alpha) {
            alpha = best;
        }
        if (alpha >= beta) {
            break;
        }
    }

    return best;
}
int findBest(Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int moveCount, int threads, int maxDepth, int timeBudgetMs) {
    if (moveCount <= 0) {
        return -1;
    }

    long startTime = getTimeMs();
    int bestIndex = 0;

    for (int depth = 1; depth <= maxDepth; depth++) {
        long elapsedBeforeDepth = getTimeMs() - startTime;
        if (timeBudgetMs > 0 && elapsedBeforeDepth >= timeBudgetMs) {
            break;
        }

        // Простая эвристика: если прошлая глубина заняла X мс, следующая обычно займёт ~X*5-8 (branching factor)
        // Если даже пессимистичная оценка "не влезет" — не начинаем новую глубину
        if (timeBudgetMs > 0 && depth > 1) {
            long estimatedNext = elapsedBeforeDepth * 6; // грубая оценка роста
            if (estimatedNext > timeBudgetMs * 3) { // с запасом, чтобы не обрубать слишком рано
                break;
            }
        }

        nodeCount = 0;
        int currentBestIndex = 0;
        int bestScore = INT_MIN;

        for (int i = 0; i < moveCount; i++) {
            Board copy = *board;
            modifyBoard(&copy, moves[i]);

            int score = -negamax(&copy, depth - 1, INT_MIN + 1, INT_MAX - 1, 1);
            printf("%s:%d\n",moves[i],score);

            if (score > bestScore) {
                bestScore = score;
                currentBestIndex = i;
                strcpy(pvTable[0][0], moves[i]);
                memcpy(pvTable[0][1], pvTable[1][0], pvLength[1] * 6);
                pvLength[0] = pvLength[1] + 1;
            }

            // Checking times
            if (timeBudgetMs > 0) {
                long elapsedMs = getTimeMs() - startTime;
                if (elapsedMs >= timeBudgetMs) {
                    return bestIndex; // Returning best move from previous depth
                }
            }
        }

        bestIndex = currentBestIndex; // Since this depth is done we are saving the result

        // --- INFO ---
        long elapsed = getTimeMs() - startTime;
        long nps = elapsed > 0 ? (nodeCount * 1000L / elapsed) : nodeCount;

        printf("info depth %d score cp %d nodes %ld nps %ld time %ld pv",
               depth, bestScore, nodeCount, nps, elapsed);
        for (int j = 0; j < pvLength[0]; j++) {
            printf(" %s", pvTable[0][j]);
        }
        printf("\n%d\n",nodeCount);
        fflush(stdout);

        long elapsedAfter = getTimeMs() - startTime;
        if (timeBudgetMs > 0 && elapsedAfter >= timeBudgetMs) break;

    }

    return bestIndex;
}