#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct {
    // [0] - white, [1] - black
    // [0]-pawn, [1]-knight, [2]-bishop, [3]-rook, [4]-queen, [5]-king
    uint64_t pieces[2][6];

    uint64_t allWhite;
    uint64_t allBlack;

    uint64_t all;

    bool isWhiteTurn;
    uint8_t castleRights;
    int enPassant;
    int halfmoveClock;
} Board;

// Testing function to display the board state in terminal

void print_board(const Board *board) {
    printf("\n  +---+---+---+---+---+---+---+---+\n");
    // Chess board is rendered top-to-bottom: from rank 8 to rank 1
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d | ", rank + 1); // Print rank number (8...1)
        
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file; // Board array index (0...63)
            char piece_char = '.';        // Default symbol for empty square
            
            // Check all 6 piece types
            for (int type = 0; type < 6; type++) {
                uint64_t mask = 1UL << square; // Create mask for the current square
                
                // Check white pieces (Index 0)
                if (board->pieces[0][type] & mask) {
                    char white_pieces[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
                    piece_char = white_pieces[type]; // Uppercase character
                    break;
                } 
                // Check black pieces (Index 1)
                else if (board->pieces[1][type] & mask) {
                    // Order must match white_pieces to align correctly with piece types
                    char black_pieces[] = {'p', 'n', 'b', 'r', 'q', 'k'};
                    piece_char = black_pieces[type]; // Lowercase character
                    break;
                }
            }
            printf("%c | ", piece_char);
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    // Print file symbols below the board
    printf("    a   b   c   d   e   f   g   h\n\n");
    printf("    Turn: %s\n", board->isWhiteTurn ? "White" : "Black");
}

// Hardcode initial state for a new game
void setStartpos(Board *board) {
    // Clear everything first
    memset(board, 0, sizeof(Board));

    // White pieces (Index 0)
    board->pieces[0][0] = 0x000000000000FF00UL; // Pawns (2nd rank)
    board->pieces[0][1] = 0x0000000000000042UL; // Knights (b1, g1)
    board->pieces[0][2] = 0x0000000000000024UL; // Bishops (c1, f1)
    board->pieces[0][3] = 0x0000000000000081UL; // Rooks (a1, h1)
    board->pieces[0][4] = 0x0000000000000008UL; // Queen (d1)
    board->pieces[0][5] = 0x0000000000000010UL; // King (e1)

    // Black pieces (Index 1)
    board->pieces[1][0] = 0x00FF000000000000UL; // Pawns (7th rank)
    board->pieces[1][1] = 0x4200000000000000UL; // Knights (b8, g8)
    board->pieces[1][2] = 0x2400000000000000UL; // Bishops (c8, f8)
    board->pieces[1][3] = 0x8100000000000000UL; // Rooks (a8, h8)
    board->pieces[1][4] = 0x0800000000000000UL; // Queen (d8)
    board->pieces[1][5] = 0x1000000000000000UL; // King (e8)

    // Combined bitboards
    board->allWhite = 0x000000000000FFFFUL; // All white pieces (1st and 2nd ranks)
    board->allBlack = 0xFFFFFF0000000000UL; // All black pieces (7th and 8th ranks)
    board->all      = 0xFFFF00000000FFFFUL; // All pieces together

    // Game state flags
    board->isWhiteTurn = true;
    board->castleRights = 15; // 1111 in binary (all castling rights available)
    board->enPassant = -1;    // No en passant square available
    board->halfmoveClock = 0;
}

// Full parser for Forsyth-Edwards Notation (FEN) strings
void parseFen(Board *board, const char *fen) {
    // Clear the board structure completely before parsing
    memset(board, 0, sizeof(Board));

    char fen_copy[256];
    strncpy(fen_copy, fen, sizeof(fen_copy) - 1);
    fen_copy[sizeof(fen_copy) - 1] = '\0';

    // 1. Piece Placement Data
    char *token = strtok(fen_copy, " ");
    if (!token) return;

    int rank = 7;
    int file = 0;

    for (size_t i = 0; i < strlen(token); i++) {
        char c = token[i];
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += (c - '0');
        } else {
            int color = isupper(c) ? 0 : 1;
            char lower_c = tolower(c);
            int type = -1;

            if (lower_c == 'p') type = 0;
            else if (lower_c == 'n') type = 1;
            else if (lower_c == 'b') type = 2;
            else if (lower_c == 'r') type = 3;
            else if (lower_c == 'q') type = 4;
            else if (lower_c == 'k') type = 5;

            if (type != -1) {
                int square = rank * 8 + file;
                board->pieces[color][type] |= (1UL << square);
                file++;
            }
        }
    }

    // 2. Active Color
    token = strtok(NULL, " ");
    if (token) {
        board->isWhiteTurn = (token[0] == 'w');
    }

    // 3. Castling Availability
    token = strtok(NULL, " ");
    if (token) {
        board->castleRights = 0;
        if (strchr(token, 'K')) board->castleRights |= 1;
        if (strchr(token, 'Q')) board->castleRights |= 2;
        if (strchr(token, 'k')) board->castleRights |= 4;
        if (strchr(token, 'q')) board->castleRights |= 8;
    }

    // 4. En Passant Target Square
    token = strtok(NULL, " ");
    if (token && token[0] != '-') {
        int ep_file = token[0] - 'a';
        int ep_rank = token[1] - '1';
        board->enPassant = ep_rank * 8 + ep_file;
    } else {
        board->enPassant = -1;
    }

    // 5. Halfmove Clock
    token = strtok(NULL, " ");
    if (token) {
        board->halfmoveClock = atoi(token);
    }

    // Recalculate occupancy tracking bitboards
    for (int type = 0; type < 6; type++) {
        board->allWhite |= board->pieces[0][type];
        board->allBlack |= board->pieces[1][type];
    }
    board->all = board->allWhite | board->allBlack;
}

// Modifies the current board state by executing a coordinate-based UCI move string
void modifyStartpos(Board *board, char *argument) {
    // Length of a valid UCI move must be 4 or 5 characters (e.g., e2e4 or e7e8q)
    size_t len = strlen(argument);
    if (len < 4 || len > 5) return;

    // Parse coordinates from string (chars 'a'-'h' -> 0-7, '1'-'8' -> 0-7)
    int from_file = argument[0] - 'a';
    int from_rank = argument[1] - '1';
    int to_file   = argument[2] - 'a';
    int to_rank   = argument[3] - '1';

    // Out-of-bounds safety guard
    if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
        to_file < 0   || to_file > 7   || to_rank < 0   || to_rank > 7) {
        return;
    }

    int from_square = from_rank * 8 + from_file;
    int to_square   = to_rank * 8 + to_file;

    uint64_t from_mask = 1UL << from_square;
    uint64_t to_mask   = 1UL << to_square;

    int us = board->isWhiteTurn ? 0 : 1;
    int them = 1 - us;

    int moved_piece_type = -1;

    // 1. Remove the piece from the source square
    for (int type = 0; type < 6; type++) {
        if (board->pieces[us][type] & from_mask) {
            board->pieces[us][type] &= ~from_mask; // Clear bit
            moved_piece_type = type;
            break;
        }
    }

    // If source square was empty, the move string is invalid
    if (moved_piece_type == -1) return;

    // 2. Capture handling: remove opponent's piece if landing on it
    for (int type = 0; type < 6; type++) {
        if (board->pieces[them][type] & to_mask) {
            board->pieces[them][type] &= ~to_mask;
            break;
        }
    }

    // 3. Pawn Promotion handling (if 5th character is present)
    if (len == 5 && moved_piece_type == 0) { // 0 is pawn
        char promo = argument[4];
        int promo_type = 4; // Queen by default
        
        if (promo == 'n' || promo == 'N') promo_type = 1;      // Knight
        else if (promo == 'b' || promo == 'B') promo_type = 2; // Bishop
        else if (promo == 'r' || promo == 'R') promo_type = 3; // Rook

        board->pieces[us][promo_type] |= to_mask; // Spawn promoted piece
    } else {
        // Quiet move: place the same piece on the destination square
        board->pieces[us][moved_piece_type] |= to_mask;
    }

    // 4. Castling handling according to the strict UCI standard (King moves 2 squares)
    if (moved_piece_type == 5) { // 5 is King
        if (from_square == 4 && to_square == 6) { // e1g1 (White O-O)
            board->pieces[0][3] &= ~(1UL << 7); board->pieces[0][3] |= (1UL << 5); // Rook h1 -> f1
        } else if (from_square == 4 && to_square == 2) { // e1c1 (White O-O-O)
            board->pieces[0][3] &= ~(1UL << 0); board->pieces[0][3] |= (1UL << 3); // Rook a1 -> d1
        } else if (from_square == 60 && to_square == 62) { // e8g8 (Black O-O)
            board->pieces[1][3] &= ~(1UL << 63); board->pieces[1][3] |= (1UL << 61); // Rook h8 -> f8
        } else if (from_square == 60 && to_square == 58) { // e8c8 (Black O-O-O)
            board->pieces[1][3] &= ~(1UL << 56); board->pieces[1][3] |= (1UL << 59); // Rook a8 -> d8
        }
    }

    // 5. Rebuild combination tracking occupancy bitboards
    board->allWhite = 0;
    board->allBlack = 0;
    for (int type = 0; type < 6; type++) {
        board->allWhite |= board->pieces[0][type];
        board->allBlack |= board->pieces[1][type];
    }
    board->all = board->allWhite | board->allBlack;

    // 6. Alternate active side turn
    board->isWhiteTurn = !board->isWhiteTurn;
}

// Router loop processing input requests using standard UCI commands
void uci_loop(Board *board) {
    char line[2000];
    
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
            print_board(board);
        } 
        else if (strcmp(command, "go") == 0) {
            // Process evaluation request constraints. Stubbing a dummy value out for now.
            printf("bestmove e2e4\n"); 
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