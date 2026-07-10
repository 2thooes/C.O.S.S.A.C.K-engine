#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
typedef struct{
    // [0] - white, [1] - black
    // [0]-pawn, [1]-horse, [2]-bishop, [3]-rook, [4]-queen, [5]-king
    uint64_t pieces[2][6];

    uint64_t allWhite;
    uint64_t allBlack;

    uint64_t all;

    bool isWhiteTurn;
    uint8_t castleRights;
    int enPassant;
    int halfmoveClock;
} Board;
// testing function
void print_board(const Board *board) {
    printf("\n  +---+---+---+---+---+---+---+---+\n");
    
    // Шахматная доска выводится сверху вниз: от 8-й горизонтали к 1-й
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d | ", rank + 1); // Выводим номер строки (8...1)
        
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file; // Индекс клетки в массиве (0...63)
            char piece_char = '.';        // Символ по умолчанию для пустой клетки
            
            // Проверяем все 6 типов фигур
            for (int type = 0; type < 6; type++) {
                uint64_t mask = 1UL << square; // Создаем маску для текущей клетки
                
                // Проверяем белые фигуры (Индекс 0)
                if (board->pieces[0][type] & mask) {
                    char white_pieces[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
                    piece_char = white_pieces[type]; // Заглавная буква
                    break;
                } 
                // Проверяем черные фигуры (Индекс 1)
                else if (board->pieces[1][type] & mask) {
                    char black_pieces[] = {'p', 'n', 'b', 'r', 'q', 'k'};
                    piece_char = black_pieces[type]; // Маленькая буква
                    break;
                }
            }
            printf("%c | ", piece_char);
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    // Выводим символы вертикалей снизу доски
    printf("    a   b   c   d   e   f   g   h\n\n");
    printf("    Turn: %s\n", board->isWhiteTurn ? "White" : "Black");
}

void setStartpos(Board *board) {
    // Белые фигуры (Индекс 0)
    board->pieces[0][0] = 0x000000000000FF00UL; // Пешки (2-я горизонталь)
    board->pieces[0][1] = 0x0000000000000042UL; // Кони (b1, g1)
    board->pieces[0][2] = 0x0000000000000024UL; // Слоны (c1, f1)
    board->pieces[0][3] = 0x0000000000000081UL; // Ладьи (a1, h1)
    board->pieces[0][4] = 0x0000000000000008UL; // Ферзь (d1)
    board->pieces[0][5] = 0x0000000000000010UL; // Король (e1)

    // Черные фигуры (Индекс 1)
    board->pieces[1][0] = 0x00FF000000000000UL; // Пешки (7-я горизонталь)
    board->pieces[1][1] = 0x4200000000000000UL; // Кони (b8, g8)
    board->pieces[1][2] = 0x2400000000000000UL; // Слоны (c8, f8)
    board->pieces[1][3] = 0x8100000000000000UL; // Ладьи (a8, h8)
    board->pieces[1][4] = 0x0800000000000000UL; // Ферзь (d8)
    board->pieces[1][5] = 0x1000000000000000UL; // Король (e8)

    // Комбинированные битборды (тоже жестко зашиваем стартовые значения)
    board->allWhite = 0x000000000000FFFFUL; // Все белые (1-я и 2-я горизонтали)
    board->allBlack = 0xFFFFFF0000000000UL; // Все черные (7-я и 8-я горизонтали)
    board->all      = 0xFFFF00000000FFFFUL; // Вообще все фигуры вместе

    // Состояние игры
    board->isWhiteTurn = true;
    board->castleRights = 15; // 1111 в двоичной (все рокировки доступны)
    board->enPassant = -1;    // Нет поля для взятия на проходе
    board->halfmoveClock = 0;
}

void modifyStartpos(){}

// function which working with uci interface
void uci_loop(Board *board) {
    char line[2000];
    
    // Отключаем буферизацию printf, чтобы GUI мгновенно получал ответы движка
    setbuf(stdout, NULL); 
    setbuf(stdin, NULL);

    while (1) {
        // Читаем строку из стандартного ввода (от GUI)
        if (!fgets(line, sizeof(line), stdin)) {
            continue;
        }

        // Убираем символ переноса строки \n в конце, если он есть
        line[strcspn(line, "\n")] = 0;

        // Если команда пустая — пропускаем
        if (line[0] == '\0') continue;

        // Берем первое слово из строки
        char *command = strtok(line, " ");

        if (strcmp(command, "uci") == 0) {
            // GUI asking engine 
            printf("id name C.O.S.S.A.C.K.\n");
            printf("id author T.Mikhaliev\n");
            // options
            printf("uciok\n");
        } 
        else if (strcmp(command, "isready") == 0) {
            // GUI проверяет, не завис ли движок
            printf("readyok\n");
        } 
        else if (strcmp(command, "ucinewgame") == 0) {
            // Начинается новая партия (очищаем доску, сбрасываем историю)
            // Тут будет вызов функции обнуления/инициализации доски
        } 
        else if (strcmp(command, "position") == 0) {
            char *argument = strtok(NULL, " ");
            
            // Безопасная проверка: argument не NULL и равен "startpos"
            if (argument && strcmp(argument, "startpos") == 0) {
                printf("info string making starting pos\n");
                setStartpos(board);
            }
            // Безопасная проверка для FEN
            else if (argument && strcmp(argument, "fen") == 0) {
                printf("info string making fen pos\n");
                // Сюда позже допишем парсер FEN
                continue; 
            }

            // Берем следующее слово. Если ходов нет, argument станет NULL
            argument = strtok(NULL, " ");
            
            // Безопасная проверка: если слово "moves" существует
            if (argument && strcmp(argument, "moves") == 0) {
                printf("info string there are some moves here!\n");
                
                // Крутим цикл, пока ходы не кончатся (argument не станет NULL)
                while ((argument = strtok(NULL, " ")) != NULL) {
                    // Передаем каждый ход в нашу функцию движения фигур
                    //parse_and_make_move(board, argument);
                }
            }
            
            // Выводим доску ОДИН РАЗ в самом конце, когда все ходы сделаны
            print_board(board);
        } 
        else if (strcmp(command, "go") == 0) {
            // GUI говорит: "Думай!". Например: "go depth 6" или "go wtime 300000"
            // Тут мы будем запускать наш Alpha-Beta поиск. 
            // Пока движок ничего не умеет, просто вернем заглушку (первый попавшийся ход)
            printf("bestmove e2e4\n"); 
        } 
        else if (strcmp(command, "quit") == 0) {
            // GUI закрывает программу
            break;
        }
    }
}

int main(){
    printf("C.O.S.S.A.C.K. Structure initialized successfully!\n");
    Board board;

    memset(&board,0,sizeof(Board));

    uci_loop(&board);

    return 0;
}