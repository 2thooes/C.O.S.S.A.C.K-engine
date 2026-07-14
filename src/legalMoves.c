#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "legalMoves.h"



// Маски вертикалей, чтобы пешки не перепрыгивали через край доски
const uint64_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFEUL;
const uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7FUL;

// Вспомогательная функция перевода координат в строку UCI (например, 12, 20 -> "e2e3")
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
        str[idx++] = 'q'; // По умолчанию генерируем превращение в ферзя (для простоты)
    }
    str[idx] = '\0';
}



void legalMovesSearch(const Board *board, char moves[MAX_MOVES][MOVE_STR_LEN], int *moveCount) {
    *moveCount = 0;
    uint64_t empty_squares = ~board->all;

    if (board->isWhiteTurn) {
        printf("Generating legal moves for white...\n");
        
        uint64_t white_pawns = board->pieces[0][0];

        // --- 1. ХОД НА 1 КЛЕТКУ ВПЕРЕД ---
        // Сдвигаем пешки вверх и проверяем, чтобы конечные поля были пусты
        uint64_t single_pushes = (white_pawns << 8) & empty_squares;

        // --- 2. ХОД НА 2 КЛЕТКИ ВПЕРЕД ---
        // Только со 2-й горизонтали (Rank 3 после первого шага) и если оба поля впереди свободны
        uint64_t rank_4_mask = 0x00000000FF000000UL; // Маска 4-й горизонтали
        uint64_t double_pushes = (single_pushes << 8) & empty_squares & rank_4_mask;

        // --- 3. ВЗЯТИЯ ПО ДИАГОНАЛИ ВПРАВО (e2 -> f3, это сдвиг << 9) ---
        // Пешка не должна стоять на вертикали H, а на целевом поле должна быть черная фигура
        uint64_t attacks_right = ((white_pawns & NOT_H_FILE) << 9) & board->allBlack;

        // --- 4. ВЗЯТИЯ ПО ДИАГОНАЛИ ВЛЕВО (e2 -> d3, это сдвиг << 7) ---
        // Пешка не должна стоять на вертикали A, а на целевом поле должна быть черная фигура
        uint64_t attacks_left = ((white_pawns & NOT_A_FILE) << 7) & board->allBlack;

        // --- 5. ВЗЯТИЕ НА ПРОХОДЕ (EN PASSANT) ---
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
    else {
        printf("generating legal moves for black ");
        // Черные пешки будут рассчитываться зеркально (сдвиги >> 8, >> 7, >> 9)
    }
}