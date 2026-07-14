#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "board.h"
#include "legalMoves.h"
// Router loop processing input requests using standard UCI commands
void uci_loop(Board *board) {
    char line[2000];
    //char *legalMoves[512];
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
                    modifyStartpos(board, argument);
                }
            }
            
            // Render the final output view representation once evaluation finishes
            printBoard(board);
        } 
        else if (strcmp(command, "go") == 0) {
            // Process evaluation request constraints. Stubbing a dummy value out for now.
            int moves = 0;
            legalMovesSearch(board,legalMoves,&moves);
            printf("bestmove e2e4\n"); 
            printf("info string Found %d legal moves\n", moves);
            
            for (int i = 0; i < moves; i++) {
                printf("info string Move %d: %s\n", i + 1, legalMoves[i]);
            } 
        } 
        else if (strcmp(command, "quit") == 0) {
            // Gracefully terminate active loop process instance
            break;
        }
    }
}



int main() {
    printf("C.O.S.S.A.C.K. Structure initialized successfully!\n");
    Board board;

    memset(&board, 0, sizeof(Board));
    uci_loop(&board);

    return 0;
}