// board.c
#include "board.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void printBoard(const Board *board) {
    printf("\n  +---+---+---+---+---+---+---+---\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d | ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            char piece_char = '.';
            
            for (int type = 0; type < 6; type++) {
                uint64_t mask = 1UL << square;
                if (board->pieces[0][type] & mask) {
                    char white_pieces[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
                    piece_char = white_pieces[type];
                    break;
                } 
                else if (board->pieces[1][type] & mask) {
                    char black_pieces[] = {'p', 'n', 'b', 'r', 'q', 'k'};
                    piece_char = black_pieces[type];
                    break;
                }
            }
            printf("%c | ", piece_char);
        }
        printf("\n  +---+---+---+---+---+---+---+---\n");
    }
    printf("    a   b   c   d   e   f   g   h\n\n");
    printf("    Turn: %s\n", board->isWhiteTurn ? "White" : "Black");
}

void setStartpos(Board *board) {
    memset(board, 0, sizeof(Board));

    board->pieces[0][0] = 0x000000000000FF00UL;
    board->pieces[0][1] = 0x0000000000000042UL;
    board->pieces[0][2] = 0x0000000000000024UL;
    board->pieces[0][3] = 0x0000000000000081UL;
    board->pieces[0][4] = 0x0000000000000008UL;
    board->pieces[0][5] = 0x0000000000000010UL;

    board->pieces[1][0] = 0x00FF000000000000UL;
    board->pieces[1][1] = 0x4200000000000000UL;
    board->pieces[1][2] = 0x2400000000000000UL;
    board->pieces[1][3] = 0x8100000000000000UL;
    board->pieces[1][4] = 0x0800000000000000UL;
    board->pieces[1][5] = 0x1000000000000000UL;

    board->allWhite = 0x000000000000FFFFUL;
    board->allBlack = 0xFFFFFF0000000000UL;
    board->all      = 0xFFFF00000000FFFFUL;

    board->isWhiteTurn = true;
    board->castleRights = 15;
    board->enPassant = -1;
    board->halfmoveClock = 0;
}

// Fixed name from parseFen to parse_fen
void parseFen(Board *board, const char *fen) {
    memset(board, 0, sizeof(Board));

    char fen_copy[256];
    strncpy(fen_copy, fen, sizeof(fen_copy) - 1);
    fen_copy[sizeof(fen_copy) - 1] = '\0';

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

    token = strtok(NULL, " ");
    if (token) {
        board->isWhiteTurn = (token[0] == 'w');
    }

    token = strtok(NULL, " ");
    if (token) {
        board->castleRights = 0;
        if (strchr(token, 'K')) board->castleRights |= 1;
        if (strchr(token, 'Q')) board->castleRights |= 2;
        if (strchr(token, 'k')) board->castleRights |= 4;
        if (strchr(token, 'q')) board->castleRights |= 8;
    }

    token = strtok(NULL, " ");
    if (token && token[0] != '-') {
        int ep_file = token[0] - 'a';
        int ep_rank = token[1] - '1';
        board->enPassant = ep_rank * 8 + ep_file;
    } else {
        board->enPassant = -1;
    }

    token = strtok(NULL, " ");
    if (token) {
        board->halfmoveClock = atoi(token);
    }

    for (int type = 0; type < 6; type++) {
        board->allWhite |= board->pieces[0][type];
        board->allBlack |= board->pieces[1][type];
    }
    board->all = board->allWhite | board->allBlack;
}

void modifyStartpos(Board *board, char *argument) {
    size_t len = strlen(argument);
    if (len < 4 || len > 5) return;

    int from_file = argument[0] - 'a';
    int from_rank = argument[1] - '1';
    int to_file   = argument[2] - 'a';
    int to_rank   = argument[3] - '1';

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

    for (int type = 0; type < 6; type++) {
        if (board->pieces[us][type] & from_mask) {
            board->pieces[us][type] &= ~from_mask;
            moved_piece_type = type;
            break;
        }
    }

    if (moved_piece_type == -1) return;

    for (int type = 0; type < 6; type++) {
        if (board->pieces[them][type] & to_mask) {
            board->pieces[them][type] &= ~to_mask;
            break;
        }
    }

    if (len == 5 && moved_piece_type == 0) {
        char promo = argument[4];
        int promo_type = 4;
        
        if (promo == 'n' || promo == 'N') promo_type = 1;
        else if (promo == 'b' || promo == 'B') promo_type = 2;
        else if (promo == 'r' || promo == 'R') promo_type = 3;

        board->pieces[us][promo_type] |= to_mask;
    } else {
        board->pieces[us][moved_piece_type] |= to_mask;
    }

    if (moved_piece_type == 5) {
        if (from_square == 4 && to_square == 6) {
            board->pieces[0][3] &= ~(1UL << 7); board->pieces[0][3] |= (1UL << 5);
        } else if (from_square == 4 && to_square == 2) {
            board->pieces[0][3] &= ~(1UL << 0); board->pieces[0][3] |= (1UL << 3);
        } else if (from_square == 60 && to_square == 62) {
            board->pieces[1][3] &= ~(1UL << 63); board->pieces[1][3] |= (1UL << 61);
        } else if (from_square == 60 && to_square == 58) {
            board->pieces[1][3] &= ~(1UL << 56); board->pieces[1][3] |= (1UL << 59);
        }
    }

    board->allWhite = 0;
    board->allBlack = 0;
    for (int type = 0; type < 6; type++) {
        board->allWhite |= board->pieces[0][type];
        board->allBlack |= board->pieces[1][type];
    }
    board->all = board->allWhite | board->allBlack;

    board->isWhiteTurn = !board->isWhiteTurn;
}