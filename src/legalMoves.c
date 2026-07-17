#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "legalMoves.h"

// Vertical masks
const uint64_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFEUL;
const uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7FUL;
const uint64_t NOT_AB_FILE = 0xFCFCFCFCFCFCFCFCUL;
const uint64_t NOT_GH_FILE = 0x3F3F3F3F3F3F3F3FUL;

// Helper function
void coordsToMoveStr(int from, int to, char *str, bool is_promotion) {
    int from_file = from % 8;
    int from_rank = from / 8;
    int to_file = to % 8;
    int to_rank = to / 8;

    int idx = 0;
    str[idx++] = 'a' + from_file;
    str[idx++] = '1' + from_rank;
    str[idx++] = 'a' + to_file;
    str[idx++] = '1' + to_rank;
    
    if (is_promotion) {
        str[idx++] = 'q'; // По умолчанию превращаемся в ферзя
    }
    str[idx] = '\0';
}

bool isKingSafe(const Board * board, int from, int to, bool is_promotion){
    char str[6];
    coordsToMoveStr(from, to, str, is_promotion);
    Board newBoard = *board;
    int us = board->isWhiteTurn ? 0 : 1;
    int enemy = board->isWhiteTurn ? 1 : 0;
    modifyBoard(&newBoard, str);
    return (newBoard.pieces[us][5] & getAttackedSquares(&newBoard, enemy)) == 0;
}

void whitePawns(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount){

    uint64_t empty_squares = ~board->all;
    uint64_t white_pawns = board->pieces[0][0];

    // --- 1. Move 1 ---
    uint64_t single_pushes = (white_pawns << 8) & empty_squares;

    // --- 2. Move 2 ---
    uint64_t rank_4_mask = 0x00000000FF000000UL; // Маска 4-й горизонтали
    uint64_t double_pushes = (single_pushes << 8) & empty_squares & rank_4_mask;

    // --- 3. Move to right diagonal ---
    uint64_t attacks_right = ((white_pawns & NOT_H_FILE) << 9) & board->allBlack;

    // --- 4. Move to left diagonal ---
    uint64_t attacks_left = ((white_pawns & NOT_A_FILE) << 7) & board->allBlack;

    // --- 5. EN PASSANT ---
    if (board->enPassant != -1) {
        uint64_t ep_mask = 1UL << board->enPassant;

        uint64_t ep_attacks_right = ((white_pawns & NOT_H_FILE) << 9) & ep_mask;
        uint64_t ep_attacks_left  = ((white_pawns & NOT_A_FILE) << 7) & ep_mask;
        
        attacks_right |= ep_attacks_right;
        attacks_left  |= ep_attacks_left;
    }
    // ==========================================
    // Translating Bitboards into strings
    // ==========================================

    uint64_t temp = single_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp); // Находим индекс установленного бита (0..63)
        int from_square = to_square - 8;
        
        bool is_promo = (to_square >= 56); // Если дошли до 8-й горизонтали — это промоушен
        if(isKingSafe(board,from_square,to_square,is_promo)){
            if(is_promo){
                coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
                (*moveCount)++;
                coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
                moves[*moveCount][4] = 'n';
                (*moveCount)++;
            }
            else{
                coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
                (*moveCount)++;
            }
        }
        
        temp &= temp - 1; 
    }

    temp = double_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square - 16;
        if(isKingSafe(board,from_square,to_square,false)){
            coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
            (*moveCount)++;
        }
        temp &= temp - 1;
    }

    temp = attacks_right;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square - 9;
        
        bool is_promo = (to_square >= 56);
        if(isKingSafe(board,from_square,to_square,is_promo)){
        if(is_promo){
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            moves[*moveCount][4] = 'n';
            (*moveCount)++;
        }
        else{
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
        }
        }
        temp &= temp - 1;
    }

    temp = attacks_left;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square - 7;
        
        bool is_promo = (to_square >= 56);
        if(isKingSafe(board,from_square,to_square,is_promo)){
        if(is_promo){
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            moves[*moveCount][4] = 'n';
            (*moveCount)++;
        }
        else{
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
        }
        }
        temp &= temp - 1;
    }
}

void blackPawns(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount){
    uint64_t empty_squares = ~board->all;
    uint64_t black_pawns = board->pieces[1][0];

    // --- 1. Move 1 ---
    uint64_t single_pushes = (black_pawns >> 8) & empty_squares;

    // --- 2. Move 2 ---
    uint64_t rank_5_mask = 0x000000FF00000000UL; // Маска 5-й горизонтали
    uint64_t double_pushes = (single_pushes >> 8) & empty_squares & rank_5_mask;

    // --- 3. Move to right diagonal ---
    uint64_t attacks_right = ((black_pawns & NOT_H_FILE) >> 7) & board->allWhite;

    // --- 4. Move to left diagonal ---
    uint64_t attacks_left = ((black_pawns & NOT_A_FILE) >> 9) & board->allWhite;

    // --- 5. EN PASSANT ---
    if (board->enPassant != -1) {
        uint64_t ep_mask = 1UL << board->enPassant;

        uint64_t ep_attacks_right = ((black_pawns & NOT_H_FILE) >> 7) & ep_mask;
        uint64_t ep_attacks_left  = ((black_pawns & NOT_A_FILE) >> 9) & ep_mask;
        
        attacks_right |= ep_attacks_right;
        attacks_left  |= ep_attacks_left;
    }
    // ==========================================
    // Translating Bitboards into strings
    // ==========================================

    uint64_t temp = single_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square + 8;
        
        bool is_promo = (to_square <= 7);
        if(isKingSafe(board,from_square,to_square,is_promo)){
        if(is_promo){
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            moves[*moveCount][4] = 'n';
            (*moveCount)++;
        }
        else{
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
        }
        }
        temp &= temp - 1; 
    }

    temp = double_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square + 16;
        if(isKingSafe(board,from_square,to_square,false)){
        coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
        (*moveCount)++;
        }
        temp &= temp - 1;
    }

    temp = attacks_right;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square + 7;
        
        bool is_promo = (to_square <= 7);
        if(isKingSafe(board,from_square,to_square,is_promo)){
        if(is_promo){
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            moves[*moveCount][4] = 'n';
            (*moveCount)++;
        }
        else{
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
        }
        }
        temp &= temp - 1;
    }

    temp = attacks_left;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square + 9;
        
        bool is_promo = (to_square <= 7);
        if(isKingSafe(board,from_square,to_square,is_promo)){
        if(is_promo){
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            moves[*moveCount][4] = 'n';
            (*moveCount)++;
        }
        else{
            coordsToMoveStr(from_square, to_square, moves[*moveCount], is_promo);
            (*moveCount)++;
        }
        }
        temp &= temp - 1;
    }
}

void knightMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    int us = board->isWhiteTurn ? 0 : 1;
    uint64_t knights = board->pieces[us][1]; // 
    
    uint64_t own_pieces = board->isWhiteTurn ? board->allWhite : board->allBlack;

    while (knights) {
        int from_square = __builtin_ctzll(knights);
        uint64_t knight_single = 1UL << from_square;

        // Looking for all 8 possible positions 
        uint64_t attacks = 0;

        attacks |= (knight_single << 17) & NOT_A_FILE;
        attacks |= (knight_single << 15) & NOT_H_FILE;
        attacks |= (knight_single << 10) & NOT_AB_FILE;
        attacks |= (knight_single << 6)  & NOT_GH_FILE;
        
        attacks |= (knight_single >> 15) & NOT_A_FILE;
        attacks |= (knight_single >> 17) & NOT_H_FILE;
        attacks |= (knight_single >> 6)  & NOT_AB_FILE;
        attacks |= (knight_single >> 10) & NOT_GH_FILE;

        // Only empty fields or fields with enimies are suitable
        uint64_t legal_attacks = attacks & ~own_pieces;

        // Translating into UCI 
        while (legal_attacks) {
            int to_square = __builtin_ctzll(legal_attacks);
            if(isKingSafe(board,from_square,to_square,false)){
            // Horses cannot do a promotions
            coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
            (*moveCount)++;
            }
            legal_attacks &= legal_attacks - 1;
        }

        knights &= knights - 1; 
    }
}

void bishopMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    int us = board->isWhiteTurn ? 0 : 1;
    uint64_t bishops = board->pieces[us][2]; 
    
    uint64_t own_pieces = board->isWhiteTurn ? board->allWhite : board->allBlack;
    uint64_t enemy_pieces = board->isWhiteTurn ? board->allBlack : board->allWhite;

    struct Direction {
        int shift;
        uint64_t edge_mask;
        bool is_upward; 
    } dirs[4] = {
        {9, NOT_H_FILE, true},  // Up-Right
        {7, NOT_A_FILE, true},  // Up-Left
        {7, NOT_H_FILE, false}, // Down-Right
        {9, NOT_A_FILE, false}  // Down-left
    };

    while (bishops) {
        int from_square = __builtin_ctzll(bishops);

        for (int d = 0; d < 4; d++) {
            uint64_t current_square = 1UL << from_square;
            struct Direction dir = dirs[d];

            while (true) {
                if ((current_square & dir.edge_mask) == 0) {
                    break;
                }

                // Changing squares safely
                if (dir.is_upward) {
                    current_square <<= dir.shift;
                } else {
                    current_square >>= dir.shift;
                }

                if (current_square == 0) break;

                int to_square = __builtin_ctzll(current_square);

                if (current_square & own_pieces) {
                    break;
                }
                if(isKingSafe(board,from_square,to_square,false)){
                coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
                (*moveCount)++;
                }
                if (current_square & enemy_pieces) {
                    break;
                }
            }
        }

        bishops &= bishops - 1; 
    }
}

void rookMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    int us = board->isWhiteTurn ? 0 : 1;
    uint64_t rooks = board->pieces[us][3]; 
    
    uint64_t own_pieces = board->isWhiteTurn ? board->allWhite : board->allBlack;
    uint64_t enemy_pieces = board->isWhiteTurn ? board->allBlack : board->allWhite;

    struct RookDirection {
        int shift;
        uint64_t edge_mask;
        bool is_positive; 
    } dirs[4] = {
        {8, 0xFFFFFFFFFFFFFFFFUL, true},  // Up
        {8, 0xFFFFFFFFFFFFFFFFUL, false}, // Down
        {1, NOT_H_FILE, true},            // Right
        {1, NOT_A_FILE, false}            // Left
    };

    while (rooks) {
        int from_square = __builtin_ctzll(rooks);

        for (int d = 0; d < 4; d++) {
            uint64_t current_square = 1UL << from_square;
            struct RookDirection dir = dirs[d];

            while (true) {
                // Edge checker
                if ((current_square & dir.edge_mask) == 0) {
                    break;
                }

                if (dir.is_positive) {
                    current_square <<= dir.shift;
                } else {
                    current_square >>= dir.shift;
                }

                if (current_square == 0) break;

                int to_square = __builtin_ctzll(current_square);

                if (current_square & own_pieces) {
                    break;
                }
                if(isKingSafe(board,from_square,to_square,false)){
                coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
                (*moveCount)++;
                }
                if (current_square & enemy_pieces) {
                    break;
                }
            }
        }

        rooks &= rooks - 1; 
    }
}

void queenMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    int us = board->isWhiteTurn ? 0 : 1;
    uint64_t queens = board->pieces[us][4];
    if (!queens) return;

    Board temp_board = *board;
    temp_board.pieces[us][4] = 0; // критично: убираем настоящего ферзя, чтобы не было призрака

    temp_board.pieces[us][2] = queens;
    bishopMoves(&temp_board, moves, moveCount);

    temp_board.pieces[us][2] = board->pieces[us][2];
    temp_board.pieces[us][3] = queens;
    rookMoves(&temp_board, moves, moveCount);
}

void kingMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    int us = board->isWhiteTurn ? 0 : 1;
    uint64_t king = board->pieces[us][5]; 
    if (!king) return;

    uint64_t own_pieces = board->isWhiteTurn ? board->allWhite : board->allBlack;
    int from_square = __builtin_ctzll(king);

    // 1. King moves every direction
    uint64_t attacks = 0;
    attacks |= (king << 8);
    attacks |= (king >> 8);
    attacks |= (king << 1) & NOT_A_FILE;
    attacks |= (king >> 1) & NOT_H_FILE;
    attacks |= (king << 9) & NOT_A_FILE;
    attacks |= (king << 7) & NOT_H_FILE;
    attacks |= (king >> 7) & NOT_A_FILE;
    attacks |= (king >> 9) & NOT_H_FILE;

    uint64_t legal_attacks = attacks & ~own_pieces;

    while (legal_attacks) {
        int to_square = __builtin_ctzll(legal_attacks);
        if(isKingSafe(board,from_square,to_square,false)){
        coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
        (*moveCount)++;
        }
        legal_attacks &= legal_attacks - 1;
    }

    // 2. Castling
    uint64_t all_pieces = board->all;
    
    if (board->isWhiteTurn) {
        // King side castling for white(e1g1)
        if ((board->castleRights & 1) && !(all_pieces & (1UL << 5)) && !(all_pieces & (1UL << 6))) {
            if(isKingSafe(board,4,6,false)){
                coordsToMoveStr(4, 6, moves[*moveCount], false); // e1 -> g1
                (*moveCount)++;
            }
        }
        // Queen side castling for white(e1c1)
        if ((board->castleRights & 2) && !(all_pieces & (1UL << 1)) && !(all_pieces & (1UL << 2)) && !(all_pieces & (1UL << 3))) {
            if(isKingSafe(board,4,2,false)){
                coordsToMoveStr(4, 2, moves[*moveCount], false); // e1 -> c1
                (*moveCount)++;
            }
        }
    } else {
        // King side castling for black (e8g8)
        if ((board->castleRights & 4) && !(all_pieces & (1UL << 61)) && !(all_pieces & (1UL << 62))) {
            if(isKingSafe(board,60,62,false)){
                coordsToMoveStr(60, 62, moves[*moveCount], false); // e8 -> g8
                (*moveCount)++;
            }
        }
        // Queen side castling for black (e8c8)
        if ((board->castleRights & 8) && !(all_pieces & (1UL << 57)) && !(all_pieces & (1UL << 58)) && !(all_pieces & (1UL << 59))) {
            if(isKingSafe(board,60,58,false)){
                coordsToMoveStr(60, 58, moves[*moveCount], false); // e8 -> c8
                (*moveCount)++;
            }
        }
    }
}

void legalMovesSearch(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    printf("info string DEBUG: board->enPassant = %d\n", board->enPassant);
    *moveCount = 0;
    if (board->isWhiteTurn) {
        printf("Generating legal moves for white...\n");
        whitePawns(board,moves,moveCount);
        knightMoves(board, moves, moveCount);
        bishopMoves(board,moves,moveCount);
        rookMoves(board,moves,moveCount);
        queenMoves(board,moves,moveCount);
        kingMoves(board,moves,moveCount);
    } 
    else {
        printf("generating legal moves for black ");
        blackPawns(board,moves,moveCount);
        knightMoves(board, moves, moveCount);
        bishopMoves(board,moves,moveCount);
        rookMoves(board,moves,moveCount);
        queenMoves(board,moves,moveCount);
        kingMoves(board,moves,moveCount);
    }
}