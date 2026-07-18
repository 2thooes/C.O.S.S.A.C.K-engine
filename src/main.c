#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "legalMoves.h"
#include "evaluate.h"
#include "findBest.h"
// Router loop processing input requests using standard UCI commands
void uci_loop(Board *board) {
    char line[2000];
    char legalMoves[256][6];
    // Disable standard output buffering for instant communication with GUI
    setbuf(stdout, NULL); 
    setbuf(stdin, NULL);

    while (1) {
        // Fetch raw command strings from GUI pipeline
        if (!fgets(line, sizeof(line), stdin)) {
            continue;
        }

        // Clean off line termination characters (\n)
        line[strcspn(line, "\n")] = 0;

        // Ignore completely blank operational sequences
        if (line[0] == '\0') continue;

        // Isolate primary command token
        char *command = strtok(line, " ");

        if (strcmp(command, "uci") == 0) {
            // Confirm identity handshake parameters
            printf("id name C.O.S.S.A.C.K.\n");
            printf("id author T.Mikhaliev\n");
            printf("uciok\n");
        } 
        else if (strcmp(command, "isready") == 0) {
            // Signal engine responsiveness check
            printf("readyok\n");
        } 
        else if (strcmp(command, "ucinewgame") == 0) {
            // Clear or reset engine histories for a new match context
            setStartpos(board);
        } 
        else if (strcmp(command, "position") == 0) {
            char *argument = strtok(NULL, " ");
            
            // Standard baseline initial layout request
            if (argument && strcmp(argument, "startpos") == 0) {
                printf("info string making starting pos\n");
                setStartpos(board);
                argument = strtok(NULL, " "); // Scan for "moves" string
            }
            // Parse custom fen states directly
            else if (argument && strcmp(argument, "fen") == 0) {
                printf("info string making fen pos\n");
                
                char fenBuffer[256] = "";
                // FEN contains 6 discrete sub-component fields separated by spaces
                for (int i = 0; i < 6; i++) {
                    argument = strtok(NULL, " ");
                    if (argument) {
                        strcat(fenBuffer, argument);
                        if (i < 5) strcat(fenBuffer, " ");
                    }
                }
                
                parseFen(board, fenBuffer);
                argument = strtok(NULL, " "); // Scan for "moves" string
            }

            // Execute post-setup sequence lists if provided
            if (argument && strcmp(argument, "moves") == 0) {
                printf("info string there are some moves here!\n");
                
                while ((argument = strtok(NULL, " ")) != NULL) {
                    modifyBoard(board, argument);
                }
            }
            
            // Render the final output view representation once evaluation finishes
            printBoard(board);
        } 
        else if (strcmp(command, "go") == 0) {
            int moves = 0;
            int threads = 1;
            // Default values
            int wtime = -1, btime = -1, winc = 0, binc = 0;
            int movestogo = -1, movetime = -1, fixedDepth = -1;
            bool infinite = false;

            char *arg = strtok(NULL, " ");
            while (arg != NULL) {
                if (strcmp(arg, "wtime") == 0) {
                    arg = strtok(NULL, " ");
                    if (arg) wtime = atoi(arg);
                }
                else if (strcmp(arg, "btime") == 0) {
                    arg = strtok(NULL, " ");
                    if (arg) btime = atoi(arg);
                }
                else if (strcmp(arg, "winc") == 0) {
                    arg = strtok(NULL, " ");
                    if (arg) winc = atoi(arg);
                }
                else if (strcmp(arg, "binc") == 0) {
                    arg = strtok(NULL, " ");
                    if (arg) binc = atoi(arg);
                }
                else if (strcmp(arg, "movestogo") == 0) {
                    arg = strtok(NULL, " ");
                    if (arg) movestogo = atoi(arg);
                }
                else if (strcmp(arg, "movetime") == 0) {
                    arg = strtok(NULL, " ");
                    if (arg) movetime = atoi(arg);
                }
                else if (strcmp(arg, "depth") == 0) {
                    arg = strtok(NULL, " ");
                    if (arg) fixedDepth = atoi(arg);
                }
                else if (strcmp(arg, "infinite") == 0) {
                    infinite = true;
                }
                arg = strtok(NULL, " ");
            }

            // Calculating the time budget
            int timeBudgetMs;
            int depthToUse;

            if (fixedDepth > 0) {
                depthToUse = fixedDepth;
                timeBudgetMs = -1; // Without time limit
            }
            else if (movetime > 0) {
                timeBudgetMs = movetime;
                depthToUse = 99; // Depth is restricted by time
            }
            else if (infinite) {
                timeBudgetMs = -1;
                depthToUse = 99;
            }
            else {
                // Usually GUI is restricting time
                int myTime  = board->isWhiteTurn ? wtime : btime;
                int myInc   = board->isWhiteTurn ? winc  : binc;

                if (myTime < 0) myTime = 5000;

                int movesLeft = (movestogo > 0) ? movestogo : 30; // If not set, let it be 30 lol

                // Simple rule: time/turn
                timeBudgetMs = (myTime / movesLeft) + (myInc / 2);

                if (timeBudgetMs > myTime / 2) {
                    timeBudgetMs = myTime / 2;
                }

                depthToUse = 99; 
            }

            legalMovesSearch(board, legalMoves, &moves);
            printf("info string Found %d legal moves\n", moves);
            for (int i = 0; i < moves; i++) {
                printf("info string Move %d: %s\n", i + 1, legalMoves[i]);
            }
            printf("info string eval: %d\n", evaluate(board));
            printf("info string time budget: %d ms, depth: %d\n", timeBudgetMs, depthToUse);

            int bestIdx = findBest(board, legalMoves, moves, threads, depthToUse, timeBudgetMs);
            printf("bestmove %s\n", legalMoves[bestIdx]);
        }
    }
}



int main() {
    srand((unsigned int)time(NULL));
    printf("C.O.S.S.A.C.K. Structure initialized successfully!\n");
    Board board;

    memset(&board, 0, sizeof(Board));
    uci_loop(&board);

    return 0;
}