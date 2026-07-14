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
        str[idx++] = 'q'; // By default promoting to a queen
    }
    str[idx] = '\0';
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
        // Проверяем, могут ли наши пешки побить на поле ep_mask
        uint64_t ep_attacks_right = ((white_pawns & NOT_H_FILE) << 9) & ep_mask;
        uint64_t ep_attacks_left  = ((white_pawns & NOT_A_FILE) << 7) & ep_mask;
        
        // Если взятие на проходе возможно, добавляем его в общие маски взятий
        attacks_right |= ep_attacks_right;
        attacks_left  |= ep_attacks_left;
    }
    // ==========================================
    // ПЕРЕВОД БИТБОРДОВ В СТРОКИ UCI ХОДОВ
    // ==========================================
    // Читаем ходы на 1 клетку
    uint64_t temp = single_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp); // Находим индекс установленного бита (0..63)
        int from_square = to_square - 8;
        
        bool is_promo = (to_square >= 56); // Если дошли до 8-й горизонтали — это промоушен
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
        
        
        temp &= temp - 1; // Сбрасываем обработанный бит
    }
    // Читаем ходы на 2 клетки
    temp = double_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square - 16;
        
        coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
        (*moveCount)++;
        
        temp &= temp - 1;
    }
    // Читаем взятия вправо
    temp = attacks_right;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square - 9;
        
        bool is_promo = (to_square >= 56);
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
        
        temp &= temp - 1;
    }
    // Читаем взятия влево
    temp = attacks_left;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square - 7;
        
        bool is_promo = (to_square >= 56);
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
        
        temp &= temp - 1;
    }
}

void blackPawns(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount){
    uint64_t empty_squares = ~board->all;
    uint64_t black_pawns = board->pieces[1][0]; // Индекс [1][0] — это чёрные пешки

    // --- 1. ХОД НА 1 КЛЕТКУ ВНИЗ ---
    // Сдвигаем пешки вниз и проверяем, чтобы конечные поля были пусты
    uint64_t single_pushes = (black_pawns >> 8) & empty_squares;

    // --- 2. ХОД НА 2 КЛЕТКИ ВНИЗ ---
    // Только с 7-й горизонтали (Rank 6 после первого шага) и если оба поля впереди свободны
    uint64_t rank_5_mask = 0x000000FF00000000UL; // Маска 5-й горизонтали
    uint64_t double_pushes = (single_pushes >> 8) & empty_squares & rank_5_mask;

    // --- 3. ВЗЯТИЯ ПО ДИАГОНАЛИ ВПРАВО (например, d7 -> e6, это сдвиг >> 7) ---
    // Пешка не должна стоять на вертикали H, а на целевом поле должна быть белая фигура
    uint64_t attacks_right = ((black_pawns & NOT_H_FILE) >> 7) & board->allWhite;

    // --- 4. ВЗЯТИЯ ПО ДИАГОНАЛИ ВЛЕВО (например, d7 -> c6, это сдвиг >> 9) ---
    // Пешка не должна стоять на вертикали A, а на целевом поле должна быть белая фигура
    uint64_t attacks_left = ((black_pawns & NOT_A_FILE) >> 9) & board->allWhite;

    // --- 5. ВЗЯТИЕ НА ПРОХОДЕ (EN PASSANT) ---
    if (board->enPassant != -1) {
        uint64_t ep_mask = 1UL << board->enPassant;
        // Проверяем, могут ли черные пешки побить на поле ep_mask
        uint64_t ep_attacks_right = ((black_pawns & NOT_H_FILE) >> 7) & ep_mask;
        uint64_t ep_attacks_left  = ((black_pawns & NOT_A_FILE) >> 9) & ep_mask;
        
        // Если взятие на проходе возможно, добавляем его в общие маски взятий
        attacks_right |= ep_attacks_right;
        attacks_left  |= ep_attacks_left;
    }
    // ==========================================
    // ПЕРЕВОД БИТБОРДОВ В СТРОКИ UCI ХОДОВ
    // ==========================================
    // Читаем ходы на 1 клетку
    uint64_t temp = single_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp); 
        int from_square = to_square + 8; // Сверху вниз: исходное поле больше целевого
        
        bool is_promo = (to_square <= 7); // Если дошли до 1-й горизонтали — это промоушен
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
        
        temp &= temp - 1; 
    }
    // Читаем ходы на 2 клетки
    temp = double_pushes;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square + 16; // Исходное поле выше на 16 позиций
        
        coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
        (*moveCount)++;
        
        temp &= temp - 1;
    }
    // Читаем взятия вправо
    temp = attacks_right;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square + 7; // Зеркально сдвигу >> 7
        
        bool is_promo = (to_square <= 7);
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
        
        temp &= temp - 1;
    }
    // Читаем взятия влево
    temp = attacks_left;
    while (temp) {
        int to_square = __builtin_ctzll(temp);
        int from_square = to_square + 9; // Зеркально сдвигу >> 9
        
        bool is_promo = (to_square <= 7);
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
        
        temp &= temp - 1;
    }
}

void KnightMoves(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    int us = board->isWhiteTurn ? 0 : 1;
    uint64_t knights = board->pieces[us][1]; // Индекс [1] — это обычно кони (P=0, N=1, B=2, R=3, Q=4, K=5)
    
    // Маска фигур нашей стороны (на них прыгать нельзя)
    uint64_t own_pieces = board->isWhiteTurn ? board->allWhite : board->allBlack;

    // Перебираем каждого коня по отдельности, чтобы знать from_square
    while (knights) {
        int from_square = __builtin_ctzll(knights);
        uint64_t knight_single = 1UL << from_square;

        // Собираем все 8 возможных прыжков для этого конкретного коня
        uint64_t attacks = 0;

        attacks |= (knight_single << 17) & NOT_A_FILE;
        attacks |= (knight_single << 15) & NOT_H_FILE;
        attacks |= (knight_single << 10) & NOT_AB_FILE;
        attacks |= (knight_single << 6)  & NOT_GH_FILE;
        
        attacks |= (knight_single >> 15) & NOT_A_FILE;
        attacks |= (knight_single >> 17) & NOT_H_FILE;
        attacks |= (knight_single >> 6)  & NOT_AB_FILE;
        attacks |= (knight_single >> 10) & NOT_GH_FILE;

        // Нам подходят только пустые поля или поля с фигурами врага
        uint64_t legal_attacks = attacks & ~own_pieces;

        // Переводим биты в UCI строки ходов
        while (legal_attacks) {
            int to_square = __builtin_ctzll(legal_attacks);
            
            // У коней промоушена не бывает, передаем false
            coordsToMoveStr(from_square, to_square, moves[*moveCount], false);
            (*moveCount)++;

            legal_attacks &= legal_attacks - 1;
        }

        knights &= knights - 1; // Сбрасываем обработанного коня
    }
}

void legalMovesSearch(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    printf("info string DEBUG: board->enPassant = %d\n", board->enPassant);
    *moveCount = 0;
    if (board->isWhiteTurn) {
        printf("Generating legal moves for white...\n");
        whitePawns(board,moves,moveCount);
        KnightMoves(board, moves, moveCount);
    } 
    else {
        printf("generating legal moves for black ");
        blackPawns(board,moves,moveCount);
        KnightMoves(board, moves, moveCount);
    }
}